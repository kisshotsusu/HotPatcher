// 版权：依据 imzlp 文章 12188（https://imzlp.com/posts/12188/）重建实现。
// 运行时 Pak 创建 / 合并。仅在 HDIFFPATCHUE_ENABLE_PAK_REBUILD=1 时编译。
//
// >>> 使用前请针对你的 UE 5.8 引擎头文件核对 <<<
// 本实现用到 FPakFile::EncodePakEntriesIntoIndex、FPakInfo、FPakEntry 及 FPakFile 枚举。
// 这些接口签名/成员会随 UE 版本漂移。下方调用沿用文章中的 UE5 签名；若 UE 5.8 不同，
// 请调整文中标注处。拿不准时请保持该宏为 0：核心的二进制差分功能（文章 25136）不依赖本文件。

#if HDIFFPATCHUE_ENABLE_PAK_REBUILD

#include "BinaryMerge.h"
#include "IPlatformFilePak.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/SecureHash.h"
#include "Misc/FileHelper.h"
#include "Serialization/MemoryWriter.h"

bool FBinaryMerge::BuildPak(const FString& OutputPak, const TArray<FBinaryMergePakEntry>& Entries,
                            const FString& MountPoint)
{
	if (Entries.Num() == 0)
	{
		return false;
	}

	// ---- 1) 序列化文件内容；为每个文件构造 FPakEntry ----
	TArray<FPakEntry> PakEntries;
	PakEntries.Reserve(Entries.Num());

	TArray<uint8> Body;
	for (const FBinaryMergePakEntry& E : Entries)
	{
		FPakEntry Entry;
		Entry.Offset = 0;                 // 真实偏移位于索引中，下方“索引前区域”再赋值
		Entry.Size = E.Data.Num();
		Entry.UncompressedSize = E.Data.Num();
		Entry.CompressionMethod = NAME_None;   // UE5 中为 FName；更老引擎是 uint8 索引，需适配
		Entry.Flags = FPakEntry::Flag_None;    // 请核对 UE 5.8 的枚举名（FPakEntry::FlagType）
		FSHA1::HashBuffer(E.Data.GetData(), static_cast<uint64>(E.Data.Num()), Entry.Hash.Hash);

		Entry.Offset = Body.Num();        // content offset within the pre-index region
		Body.Append(E.Data);
		PakEntries.Add(Entry);
	}

	// ---- 2) 构造 FPakInfo 并对索引编码 ----
	FPakInfo Info;
	Info.Magic = FPakInfo::PakFile_Magic;
	Info.Version = FPakInfo::PakFile_Version_Latest;   // or a specific FPakInfo::PakFile_Version_* if needed
	Info.bEncryptedIndex = 0;
	Info.bIndexIsFrozen = 0;
	Info.EncryptionKeyGuid = FGuid();
	Info.CompressionMethods.Empty();
	Info.CompressionMethods.Add(FName("None"));

	TArray<uint8> EncodedIndex;
	TArray<FPakEntry> NonEncodableEntries;
	FDirectoryIndex* DirectoryIndex = nullptr;
	FPathHashIndex* PathHashIndex = nullptr;
	uint64 PathHashSeed = 0;
	int32 NumEncodedEntries = 0;
	int32 NumDeletedEntries = 0;

	// ReadNextEntry 回调：(int32 Index, FPakEntry& OutEntry, FString& OutPath, bool& OutIsDeleted)
	auto ReadNextEntry = [&](int32 Index, FPakEntry& OutEntry, FString& OutPath, bool& OutIsDeleted)
	{
		OutEntry = PakEntries[Index];
		OutPath = Entries[Index].MountPath;
		OutIsDeleted = false;
	};

	FPakFile::EncodePakEntriesIntoIndex(
		PakEntries.Num(),
		ReadNextEntry,
		*OutputPak,
		Info,
		MountPoint,
		NumEncodedEntries,
		NumDeletedEntries,
		&PathHashSeed,
		DirectoryIndex,
		PathHashIndex,
		EncodedIndex,
		NonEncodableEntries,
		nullptr,            // collision detection map (optional)
		Info.Version);      // PakFileVersion (last param in the article's UE5 signature)

	// ---- 3) 定稿索引偏移/大小/哈希，然后写出 [Body][EncodedIndex][FPakInfo] ----
	Info.IndexOffset = Body.Num();
	Info.IndexSize = EncodedIndex.Num();
	FSHA1::HashBuffer(EncodedIndex.GetData(), static_cast<uint64>(EncodedIndex.Num()), Info.IndexHash.Hash);

	TArray<uint8> Out;
	Out.Append(Body);
	Out.Append(EncodedIndex);

	// FPakInfo is serialized at the very end of the file (verify FPakInfo::Serialize signature on 5.8).
	FMemoryWriter Writer(Out, true);
	Info.Serialize(Writer, Info.Version);
	Writer.Close();

	return FFileHelper::SaveArrayToFile(Out, *OutputPak);
}

bool FBinaryMerge::MergePaks(const FString& OutputPak, const TArray<FString>& InputPaks, const FString& MountPoint)
{
	TArray<FBinaryMergePakEntry> All;
	for (const FString& PakPath : InputPaks)
	{
		if (!FPaths::FileExists(PakPath))
		{
			continue;
		}
		TUniquePtr<FPakFile> Pak(new FPakFile(*PakPath, false));
		if (!Pak || !Pak->IsValid())
		{
			continue;
		}
		// 枚举源 pak 中的每个文件（请核对 UE 5.8 的 GetFileList 名称）。
		TArray<FString> FileList;
		Pak->GetFileList(FileList);
		for (const FString& Path : FileList)
		{
			TArray<uint8> Bytes;
			if (Pak->ReadFile(*Path, Bytes))   // 请核对 UE 5.8 的 ReadFile 签名
			{
				FBinaryMergePakEntry E;
				E.MountPath = Path;
				E.Data = MoveTemp(Bytes);
				All.Add(MoveTemp(E));
			}
		}
	}
	if (All.Num() == 0)
	{
		return false;
	}
	return BuildPak(OutputPak, All, MountPoint);
}

#else // !HDIFFPATCHUE_ENABLE_PAK_REBUILD

#include "BinaryMerge.h"

bool FBinaryMerge::BuildPak(const FString& OutputPak, const TArray<FBinaryMergePakEntry>& Entries,
                            const FString& MountPoint)
{
	UE_LOG(LogTemp, Warning,
		TEXT("FBinaryMerge::BuildPak is disabled. Set HDIFFPATCHUE_ENABLE_PAK_REBUILD=1 in HDiffPatchUE.Build.cs "
		     "and verify the UE 5.8 Pak API in PakRebuilder.cpp (see https://imzlp.com/posts/12188/)."));
	return false;
}

bool FBinaryMerge::MergePaks(const FString& OutputPak, const TArray<FString>& InputPaks, const FString& MountPoint)
{
	UE_LOG(LogTemp, Warning,
		TEXT("FBinaryMerge::MergePaks is disabled. Set HDIFFPATCHUE_ENABLE_PAK_REBUILD=1 in HDiffPatchUE.Build.cs."));
	return false;
}

#endif // HDIFFPATCHUE_ENABLE_PAK_REBUILD
