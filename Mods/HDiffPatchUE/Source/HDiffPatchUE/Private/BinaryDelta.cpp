// Copyright (c) rebuilt per imzlp article 25136. See BinaryDelta.h.
#include "BinaryDelta.h"
#include "HAL/UnrealMemory.h"
#include "Math/UnrealMathUtility.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/Paths.h"

bool FBinaryDelta::CreateDiff(const TArray<uint8>& NewData, const TArray<uint8>& OldData, TArray<uint8>& OutPatch)
{
	// 硬性上限：本实现把 patch 与中间缓冲都放在 TArray<uint8>（int32 索引，上限约 2 GiB）。
	// 单个文件达到该量级时 Reserve/Append 会整数溢出并触发 TArray 断言崩溃，而非“内存不足”。
	// 因此这里在入口安全退出。NOTE: 整 pak/utoc（数 GiB）的差分需要分块流式 CreateDiff，
	// 当前编辑器侧产出 .patch 不支持 >~2GiB 单文件；运行时 apply（PatchDiffToFile）无此限制。
	static const int64 kMaxFileBytes = static_cast<int64>(MAX_int32) - 1024;
	if (NewData.Num() > kMaxFileBytes || OldData.Num() > kMaxFileBytes)
	{
		UE_LOG(LogTemp, Error,
			TEXT("BinaryDelta::CreateDiff: 单个文件超过约 2 GiB 上限（New=%lld, Old=%lld），"
			     "TArray<uint8> 为 int32 索引无法容纳，已安全退出（未崩溃）。整容器差分需流式 CreateDiff。"),
			static_cast<int64>(NewData.Num()), static_cast<int64>(OldData.Num()));
		return false;
	}

	OutPatch.Empty();
	OutPatch.Reserve(NewData.Num() + 64);

	FBinaryDeltaHeader Header;
	Header.Magic   = BinaryDeltaPrivate::kBDifMagic;
	Header.Version = BinaryDeltaPrivate::kBDifVersion;
	Header.OldSize = (uint64)OldData.Num();
	Header.NewSize = (uint64)NewData.Num();
	OutPatch.Append(reinterpret_cast<const uint8*>(&Header), sizeof(Header));

	const int32 BlockSize = BinaryDeltaPrivate::kBlockSize;
	const int32 MinMatch  = BinaryDeltaPrivate::kMinMatch;
	const uint8* Old = OldData.GetData();
	const uint8* New = NewData.GetData();
	const int64 OldNum = OldData.Num();
	const int64 NewNum = NewData.Num();

	// Index of Old: first position of each BlockSize-window hash. Bounds memory to ~|Old|/BlockSize entries.
	// NOTE: 只存每个哈希的【首次】出现位置。多项式哈希为 32 位，对大 Old（>2GiB 有数百万窗口）碰撞概率不低；
	// 碰撞时真实可 COPY 的块会被当 INSERT，patch 体积膨胀。逐字节扩展（下方 while 循环）保证不会产生错误
	// 数据，仅压缩率退化（明显弱于真 HDiffPatch）。若要逼近其压缩率，需更强哈希或全位置索引+最长匹配。
	TMap<uint32, int64> OldHashToPos;
	if (OldNum >= BlockSize)
	{
		OldHashToPos.Reserve(static_cast<int32>(FMath::Min<int64>(OldNum / BlockSize + 1, 8 * 1024 * 1024)));
		for (int64 i = 0; i + BlockSize <= OldNum; ++i)
		{
			const uint32 h = BinaryDeltaPrivate::HashBlock(Old, i, BlockSize);
			if (!OldHashToPos.Contains(h))
			{
				OldHashToPos.Add(h, i);
			}
		}
	}

	auto EmitCopy = [&](int64 Offset, int64 Length)
	{
		uint8 Buf[16];
		OutPatch.Add(0x01); // OP_COPY
		FMemory::Memcpy(&Buf[0], &Offset, 8);   // little-endian on LE platforms; see header note
		FMemory::Memcpy(&Buf[8], &Length, 8);
		OutPatch.Append(Buf, 16);
	};
	auto EmitInsert = [&](const uint8* Ptr, int32 Length)
	{
		uint8 Buf[4];
		OutPatch.Add(0x02); // OP_INSERT
		const uint32 Len32 = static_cast<uint32>(Length);
		FMemory::Memcpy(&Buf[0], &Len32, 4);
		OutPatch.Append(Buf, 4);
		OutPatch.Append(Ptr, Length);
	};

	uint32 CurrentHash = 0;

	TArray<uint8> Pending; // buffered literal bytes between copies
	auto FlushPending = [&]()
	{
		if (Pending.Num() > 0)
		{
			EmitInsert(Pending.GetData(), Pending.Num());
			Pending.Reset();
		}
	};

	int64 Pos = 0;
	while (Pos < NewNum)
	{
		bool bMatched = false;
		if (Pos + BlockSize <= NewNum && OldNum >= BlockSize)
		{
			if (Pos == 0)
			{
				CurrentHash = BinaryDeltaPrivate::HashBlock(New, 0, BlockSize);
			}
			else
			{
				CurrentHash = BinaryDeltaPrivate::RollHash(CurrentHash, New[Pos - 1], New[Pos + BlockSize - 1]);
			}

			const int64* OldPosPtr = OldHashToPos.Find(CurrentHash);
			if (OldPosPtr)
			{
				const int64 OldPos = *OldPosPtr;
				// Verify the actual bytes — rolling-hash collisions must not corrupt data.
				if (FMemory::Memcmp(New + Pos, Old + OldPos, BlockSize) == 0)
				{
					const int64 MaxLen = FMath::Min(NewNum - Pos, OldNum - OldPos);
					int64 MatchLen = BlockSize;
					while (MatchLen < MaxLen && New[Pos + MatchLen] == Old[OldPos + MatchLen])
					{
						++MatchLen;
					}
					if (MatchLen >= MinMatch)
					{
						FlushPending();
						EmitCopy(OldPos, MatchLen);
						Pos += MatchLen;
						bMatched = true;
					}
				}
			}
		}
		if (!bMatched)
		{
			Pending.Add(New[Pos]);
			++Pos;
		}
	}
	FlushPending();
	return true;
}

bool FBinaryDelta::PatchDiff(const TArray<uint8>& OldData, const TArray<uint8>& PatchData, TArray<uint8>& OutNewData)
{
	OutNewData.Empty();

	if (PatchData.Num() < static_cast<int32>(sizeof(FBinaryDeltaHeader)))
	{
		return false;
	}
	FBinaryDeltaHeader Header;
	FMemory::Memcpy(&Header, PatchData.GetData(), sizeof(Header));
	if (Header.Magic != BinaryDeltaPrivate::kBDifMagic || Header.Version != BinaryDeltaPrivate::kBDifVersion)
	{
		return false;
	}
	if (Header.NewSize > static_cast<uint64>(MAX_int32) || Header.OldSize > static_cast<uint64>(MAX_int32))
	{
		// UE TArray<uint8> is int32-indexed; files above 2 GiB cannot be reconstructed here.
		return false;
	}
	if (static_cast<uint64>(OldData.Num()) != Header.OldSize)
	{
		// Base file does not match what the patch was created against -> cannot rebuild.
		UE_LOG(LogTemp, Warning,
			TEXT("BinaryDelta::PatchDiff: base file size mismatch (patch expects %llu bytes, got %llu). "
			     "The .patch was generated from a different base version."),
			static_cast<unsigned long long>(Header.OldSize),
			static_cast<unsigned long long>(OldData.Num()));
		return false;
	}

	OutNewData.SetNumUninitialized(static_cast<int32>(Header.NewSize));
	uint8* Out    = OutNewData.GetData();
	const uint8* Old   = OldData.GetData();
	const uint8* Patch = PatchData.GetData();
	const int64 PatchNum = PatchData.Num();
	int64 OutPos = 0;
	int64 OpPos  = static_cast<int64>(sizeof(FBinaryDeltaHeader));

	auto ReadU64 = [&](int64 At) -> uint64 { uint64 v; FMemory::Memcpy(&v, Patch + At, 8); return v; };
	auto ReadU32 = [&](int64 At) -> uint32 { uint32 v; FMemory::Memcpy(&v, Patch + At, 4); return v; };

	while (OpPos < PatchNum)
	{
		const uint8 Tag = Patch[OpPos++];
		if (Tag == 0x01) // OP_COPY: reference a slice of Old
		{
			if (OpPos + 16 > PatchNum) return false;
			const uint64 Offset = ReadU64(OpPos);
			const uint64 Length = ReadU64(OpPos + 8);
			OpPos += 16;
			if (Offset >= static_cast<uint64>(OldData.Num())) return false;
			if (Length > static_cast<uint64>(OldData.Num()) - Offset) return false;
			if (static_cast<uint64>(OutPos) > Header.NewSize ||
			    Length > Header.NewSize - static_cast<uint64>(OutPos)) return false;

			FMemory::Memcpy(Out + OutPos, Old + static_cast<SIZE_T>(Offset), static_cast<SIZE_T>(Length));
			OutPos += static_cast<int64>(Length);
		}
		else if (Tag == 0x02) // OP_INSERT: literal bytes from the patch
		{
			if (OpPos + 4 > PatchNum) return false;
			const uint32 Length = ReadU32(OpPos);
			OpPos += 4;
			if (OpPos + static_cast<int64>(Length) > PatchNum) return false;

			if (static_cast<uint64>(OutPos) > Header.NewSize ||
			    static_cast<uint64>(Length) > Header.NewSize - static_cast<uint64>(OutPos)) return false;

			FMemory::Memcpy(Out + OutPos, Patch + OpPos, static_cast<SIZE_T>(Length));
			OpPos += static_cast<int64>(Length);
			OutPos += static_cast<int64>(Length);
		}
		else
		{
			return false; // unknown opcode -> corrupt patch
		}
	}
	return OutPos == static_cast<int64>(Header.NewSize);
}

bool FBinaryDelta::PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath)
{
	IPlatformFile& PF = FPlatformFileManager::Get().GetPlatformFile();

	TUniquePtr<IFileHandle> OldHandle(PF.OpenRead(*OldFilePath));
	if (!OldHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("BinaryDelta::PatchDiffToFile: 无法打开基础文件 %s"), *OldFilePath);
		return false;
	}
	const int64 OldFileSize = OldHandle->Size();

	TUniquePtr<IFileHandle> PatchHandle(PF.OpenRead(*PatchFilePath));
	if (!PatchHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("BinaryDelta::PatchDiffToFile: 无法打开补丁文件 %s"), *PatchFilePath);
		return false;
	}

	// 确保输出目录存在
	const FString OutDir = FPaths::GetPath(OutNewFilePath);
	if (!OutDir.IsEmpty() && !PF.DirectoryExists(*OutDir))
	{
		PF.CreateDirectoryTree(*OutDir);
	}
	TUniquePtr<IFileHandle> OutHandle(PF.OpenWrite(*OutNewFilePath, /*bAppend=*/false, /*bAllowRead=*/false));
	if (!OutHandle)
	{
		UE_LOG(LogTemp, Warning, TEXT("BinaryDelta::PatchDiffToFile: 无法创建输出文件 %s"), *OutNewFilePath);
		return false;
	}

	// 读取并校验文件头
	FBinaryDeltaHeader Header;
	{
		uint8 HdrBuf[sizeof(FBinaryDeltaHeader)];
		if (!PatchHandle->Read(HdrBuf, sizeof(HdrBuf)))
		{
			UE_LOG(LogTemp, Warning, TEXT("BinaryDelta::PatchDiffToFile: 补丁文件过小/无法读取头"));
			return false;
		}
		FMemory::Memcpy(&Header, HdrBuf, sizeof(Header));
	}
	if (Header.Magic != BinaryDeltaPrivate::kBDifMagic || Header.Version != BinaryDeltaPrivate::kBDifVersion)
	{
		UE_LOG(LogTemp, Warning, TEXT("BinaryDelta::PatchDiffToFile: 补丁魔数/版本不匹配"));
		return false;
	}
	if (static_cast<uint64>(OldFileSize) != Header.OldSize)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BinaryDelta::PatchDiffToFile: 基础文件大小不匹配（补丁期望 %llu 字节，实际 %llu）。"
			     ".patch 由不同基础版本生成。"),
			static_cast<unsigned long long>(Header.OldSize),
			static_cast<unsigned long long>(OldFileSize));
		return false;
	}

	// 固定大小工作缓冲：峰值内存约等于此值，与文件大小无关
	const int64 Chunk = 1 * 1024 * 1024; // 1 MiB
	TArray<uint8> Buf;
	Buf.SetNumUninitialized(Chunk);
	uint8* B = Buf.GetData();

	auto ReadExact = [&](IFileHandle* H, uint8* Dst, int64 Want) -> bool
	{
		return H->Read(Dst, Want);
	};
	auto WriteAll = [&](const uint8* Src, int64 Want) -> bool
	{
		return OutHandle->Write(Src, Want);
	};

	uint64 OutPos = 0;
	bool bOk = true;

	for (;;)
	{
		uint8 Tag = 0;
		if (!PatchHandle->Read(&Tag, 1)) break; // EOF or read error; validated below via size check

		if (Tag == 0x01) // OP_COPY：从基础文件按偏移分块读取
		{
			uint8 P[16];
			if (!ReadExact(PatchHandle.Get(), P, 16)) { bOk = false; break; }
			uint64 Offset;
			uint64 Length;
			FMemory::Memcpy(&Offset, &P[0], 8);
			FMemory::Memcpy(&Length, &P[8], 8);
			if (Offset >= static_cast<uint64>(OldFileSize) ||
			    Length > static_cast<uint64>(OldFileSize) - Offset) { bOk = false; break; }

			uint64 Remaining = Length;
			while (Remaining > 0)
			{
				const int64 ToRead = static_cast<int64>(FMath::Min<uint64>(Remaining, static_cast<uint64>(Chunk)));
				OldHandle->Seek(static_cast<int64>(Offset + (Length - Remaining)));
				// 与 INSERT 分支一致，用 ReadExact 处理磁盘可能的部分读，避免单次 Read 不足即误判失败
				if (!ReadExact(OldHandle.Get(), B, ToRead)) { bOk = false; break; }
				if (!WriteAll(B, ToRead)) { bOk = false; break; }
				Remaining -= static_cast<uint64>(ToRead);
			}
			if (!bOk) break;
			OutPos += Length;
		}
		else if (Tag == 0x02) // OP_INSERT：补丁内的字面量，顺序分块读取
		{
			uint8 L[4];
			if (!ReadExact(PatchHandle.Get(), L, 4)) { bOk = false; break; }
			uint32 Length;
			FMemory::Memcpy(&Length, &L[0], 4);
			uint64 Remaining = static_cast<uint64>(Length);
			while (Remaining > 0)
			{
				const int64 ToRead = static_cast<int64>(FMath::Min<uint64>(Remaining, static_cast<uint64>(Chunk)));
				if (!ReadExact(PatchHandle.Get(), B, ToRead)) { bOk = false; break; }
				if (!WriteAll(B, ToRead)) { bOk = false; break; }
				Remaining -= static_cast<uint64>(ToRead);
			}
			if (!bOk) break;
			OutPos += static_cast<uint64>(Length);
		}
		else
		{
			bOk = false; // 未知操作码 -> 补丁损坏
			break;
		}
	}

	OutHandle->Flush();

	if (!bOk || OutPos != Header.NewSize)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BinaryDelta::PatchDiffToFile: 应用失败（bOk=%d, 产出 %llu / 期望 %llu 字节）"),
			bOk ? 1 : 0,
			static_cast<unsigned long long>(OutPos),
			static_cast<unsigned long long>(Header.NewSize));
		OutHandle.Reset();
		PF.DeleteFile(*OutNewFilePath);
		return false;
	}
	return true;
}
