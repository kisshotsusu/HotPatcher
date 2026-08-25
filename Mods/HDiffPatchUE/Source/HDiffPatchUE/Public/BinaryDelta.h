// Copyright (c) rebuilt per imzlp article 25136 (https://imzlp.com/posts/25136/).
// Self-contained binary delta: a byte-level copy/insert (BSDIFF-lite) encoding.
//
// Format-agnostic: the algorithm only ever sees byte arrays, so it works for ANY file,
// including Unreal .pak containers and IoStore .utoc/.ucas pairs. "Binary file merge" here
// means: given the OLD base file and a .patch produced by CreateDiff, reconstruct the NEW file.
//
// The pair is symmetric: HotPatcher's GenerateBinariesPatch (editor) calls CreateDiff(New, Old)
// and the CloudUpdate client (packaged game) calls PatchDiff(Old, Patch) to rebuild the file.
#pragma once

#include "CoreMinimal.h"

struct FBinaryDeltaHeader
{
	uint32 Magic;   // 'BDIF'
	uint32 Version; // 1
	uint64 OldSize;
	uint64 NewSize;
};

namespace BinaryDeltaPrivate
{
	static const uint32 kBDifMagic   = 0x42444946; // 'B','D','I','F'
	static const uint32 kBDifVersion = 1;
	static const int32  kBlockSize   = 64; // hash block / minimum match window
	static const int32  kMinMatch    = 64; // a COPY is only worth emitting above this length

	FORCEINLINE uint32 HashBlock(const uint8* Data, int64 Pos, int32 Len)
	{
		// FNV-1a 32-bit over Len bytes.
		uint32 Hash = 2166136261u;
		for (int32 i = 0; i < Len; ++i)
		{
			Hash ^= Data[Pos + i];
			Hash *= 16777619u;
		}
		return Hash;
	}
}

class FBinaryDelta
{
public:
	// NewData, OldData -> OutPatch. Always succeeds (output is a valid patch for the given inputs).
	static bool CreateDiff(const TArray<uint8>& NewData, const TArray<uint8>& OldData, TArray<uint8>& OutPatch);

	// OldData, PatchData -> OutNewData. Returns false if the patch is corrupt or OldSize mismatches.
	static bool PatchDiff(const TArray<uint8>& OldData, const TArray<uint8>& PatchData, TArray<uint8>& OutNewData);

	// Streaming file-to-file apply. Reads Old and Patch via file handles in fixed-size chunks
	// (~a few MiB peak memory) and writes New to disk in chunks. Unlike PatchDiff (which buffers
	// everything in a TArray and is limited to ~2 GiB files), this handles arbitrarily large files.
	// Returns false if the patch is corrupt, the base file size mismatches the patch's OldSize,
	// or any I/O fails.
	static bool PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath);
};
