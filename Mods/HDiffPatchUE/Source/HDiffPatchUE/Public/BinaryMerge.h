// Copyright (c) rebuilt per imzlp article 12188 (https://imzlp.com/posts/12188/).
// Runtime Pak creation / merge utilities.
//
// IMPORTANT: this relies on engine-internal Pak serialization (FPakFile::EncodePakEntriesIntoIndex,
// FPakInfo). Those signatures DRIFT between UE versions. The implementation in PakRebuilder.cpp is
// compiled only when HDIFFPATCHUE_ENABLE_PAK_REBUILD=1 (see HDiffPatchUE.Build.cs). When you enable
// it, verify the EncodePakEntriesIntoIndex / FPakInfo signatures against your UE 5.8 engine headers
// and adapt the call if necessary. When disabled, BuildPak/MergePaks return false with a log notice,
// and the core binary-delta feature (article 25136) is unaffected.
#pragma once

#include "CoreMinimal.h"

// One file to be serialized into a Pak. MountPath is the UFS virtual path,
// e.g. "../../../PROJECT/Content/Paks/foo.uasset".
struct FBinaryMergePakEntry
{
	FString MountPath;
	TArray<uint8> Data;
};

struct FBinaryMerge
{
	// Build a Pak at OutputPak from in-memory entries. MountPoint is the common UFS prefix
	// stripped from each MountPath when indexing (default "../../../").
	static bool BuildPak(const FString& OutputPak, const TArray<FBinaryMergePakEntry>& Entries,
	                     const FString& MountPoint = TEXT("../../../"));

	// Merge several existing Pak files into a single OutputPak. On path conflicts the FIRST
	// input pak wins (Pak mount order semantics). Requires reading every source pak's index,
	// so it needs the same engine Pak APIs as BuildPak.
	static bool MergePaks(const FString& OutputPak, const TArray<FString>& InputPaks,
	                      const FString& MountPoint = TEXT("../../../"));
};
