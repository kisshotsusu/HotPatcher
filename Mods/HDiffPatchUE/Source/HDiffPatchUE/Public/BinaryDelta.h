// 版权：依据 imzlp 文章 25136（https://imzlp.com/posts/25136/）重建实现。
// 自包含的二进制差分：一种字节级 copy/insert（BSDIFF-lite）编码。
//
// 与格式无关：算法只处理字节数组，因此适用于任意文件，包括 Unreal 的 .pak 容器
// 与 IoStore 的 .utoc/.ucas 配对。此处的“二进制文件合并”指：给定 OLD 基础文件与
// CreateDiff 产出的 .patch，重建出 NEW 文件。
//
// 两侧对称：HotPatcher 编辑器侧 GenerateBinariesPatch 调用 CreateDiff(New, Old) 产出补丁，
// 运行时客户端（如 CloudUpdate，打包后的游戏）调用 PatchDiff(Old, Patch) 重建文件。
#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatform.h"

struct FBinaryDeltaHeader
{
	uint32 Magic;   // 'BDIF'
	uint32 Version; // 1. Low bit set = big-endian payload (0x...1); clear = little-endian.
	uint64 OldSize;
	uint64 NewSize;
};

namespace BinaryDeltaPrivate
{
	static const uint32 kBDifMagic   = 0x42444946; // 'B','D','I','F'
	// Bit 31 of Version marks big-endian payload; clear means little-endian.
	static const uint32 kBDifVersion = PLATFORM_LITTLE_ENDIAN ? 1u : 0x80000001u;
	static const int32  kBlockSize   = 64; // hash block / minimum match window
	static const int32  kMinMatch    = 64; // a COPY is only worth emitting above this length

	static constexpr uint32 kHashBase = 31u;

	// kBlockSize-th power of kHashBase, computed at compile time.
	struct FHashPow
	{
		static constexpr uint32 Compute()
		{
			uint32 Result = 1;
			for (int32 i = 0; i < kBlockSize; ++i)
			{
				Result *= kHashBase;
			}
			return Result;
		}
	};

	static constexpr uint32 kHashPow = FHashPow::Compute();

	FORCEINLINE uint32 HashBlock(const uint8* Data, int64 Pos, int32 Len)
	{
		// Polynomial rolling hash (Rabin-Karp style), uint32 natural overflow.
		uint32 Hash = 0;
		{
			for (int32 i = 0; i < Len; ++i)
			{
				Hash = Hash * kHashBase + Data[Pos + i];
			}
		}
		return Hash;
	}

	// Advance the sliding window by one byte: remove OutByte from front, add InByte at back.
	// All arithmetic uses uint32 natural overflow — no explicit modulus needed.
	FORCEINLINE uint32 RollHash(uint32 Hash, uint8 OutByte, uint8 InByte)
	{
		return (Hash - static_cast<uint32>(OutByte) * kHashPow) * kHashBase + InByte;
	}
}

class FBinaryDelta
{
public:
	// NewData, OldData -> OutPatch（产出二进制补丁）。只要输入合法，必定成功产出可用补丁。
	// 注意：单文件超过约 2 GiB 时安全退出（详见 .cpp 内的 kMaxFileBytes 上限说明）。
	static bool CreateDiff(const TArray<uint8>& NewData, const TArray<uint8>& OldData, TArray<uint8>& OutPatch);

	// OldData, PatchData -> OutNewData。补丁损坏或基础文件大小与补丁记录的 OldSize 不符时返回 false。
	// 整文件缓冲，仅支持 ~2 GiB 以下文件；更大文件请使用 PatchDiffToFile。
	static bool PatchDiff(const TArray<uint8>& OldData, const TArray<uint8>& PatchData, TArray<uint8>& OutNewData);

	// 流式按文件重建：通过文件句柄以固定大小分块读写 Old 与 Patch（峰值内存仅约数 MiB），
	// 并把 New 分块写入磁盘。与 PatchDiff（整文件缓冲、限制 ~2 GiB）不同，本函数可处理任意大文件。
	// 补丁损坏、基础文件大小与补丁的 OldSize 不符、或任意 I/O 失败时返回 false。
	static bool PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath);
};
