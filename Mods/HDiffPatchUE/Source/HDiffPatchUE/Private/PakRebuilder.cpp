// Copyright (c) rebuilt per imzlp article 12188 (https://imzlp.com/posts/12188/).
// Runtime Pak creation / merge. Compiled only when HDIFFPATCHUE_ENABLE_PAK_REBUILD=1.
//
// >>> VERIFY AGAINST YOUR UE 5.8 ENGINE HEADERS BEFORE USE <<<
// This uses FPakFile::EncodePakEntriesIntoIndex, FPakInfo, FPakEntry and FPakFile enumeration.
// Their signatures/members DRIFT between UE versions. The calls below follow the article's UE5
// signature; if UE 5.8 differs, adjust the flagged spots. When in doubt, keep the define at 0:
// the core binary-delta (article 25136) is fully functional without this file.

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

	// ---- 1) Serialize file contents; build FPakEntry per file ----
	TArray<FPakEntry> PakEntries;
	PakEntries.Reserve(Entries.Num());

	TArray<uint8> Body;
	for (const FBinaryMergePakEntry& E : Entries)
	{
		FPakEntry Entry;
		Entry.Offset = 0;                 // real offset lives in the index; set below for the pre-index body
		Entry.Size = E.Data.Num();
		Entry.UncompressedSize = E.Data.Num();
		Entry.CompressionMethod = NAME_None;   // UE5: FName; on older engines this is a uint8 index -> adapt
		Entry.Flags = FPakEntry::Flag_None;    // verify enum name on 5.8 (FPakEntry::FlagType)
		FSHA1::HashBuffer(E.Data.GetData(), static_cast<uint64>(E.Data.Num()), Entry.Hash.Hash);

		Entry.Offset = Body.Num();        // content offset within the pre-index region
		Body.Append(E.Data);
		PakEntries.Add(Entry);
	}

	// ---- 2) Build FPakInfo and encode the index ----
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

	// ReadNextEntryFunction: (int32 Index, FPakEntry& OutEntry, FString& OutPath, bool& OutIsDeleted)
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

	// ---- 3) Finalize index offset/size/hash, then write [Body][EncodedIndex][FPakInfo] ----
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
		FPakFile* Pak = new FPakFile(*PakPath, false);
		if (!Pak || !Pak->IsValid())
		{
			delete Pak;
			continue;
		}
		// Enumerate every file in the source pak (verify GetFileList name on 5.8).
		TArray<FString> FileList;
		Pak->GetFileList(FileList);
		for (const FString& Path : FileList)
		{
			TArray<uint8> Bytes;
			if (Pak->ReadFile(*Path, Bytes))   // verify ReadFile signature on 5.8
			{
				FBinaryMergePakEntry E;
				E.MountPath = Path;
				E.Data = MoveTemp(Bytes);
				All.Add(MoveTemp(E));
			}
		}
		delete Pak;
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
