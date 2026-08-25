// Copyright (c) rebuilt per imzlp articles 25136 & 12188.
// Self-contained replacement for the dead hxhb/HDiffPatchUE submodule.
using System;
using System.IO;
using UnrealBuildTool;

public class HDiffPatchUE : ModuleRules
{
	public HDiffPatchUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Public") });

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Projects",
			"BinariesPatchFeature",   // provides IBinariesDiffPatchFeature + BINARIES_DIFF_PATCH_FEATURE_NAME
			"PakFile"                 // used by the article-12188 runtime pak rebuild/merge utility
		});

		// Article 12188 runtime pak rebuild/merge utility (FBinaryMerge::BuildPak / MergePaks).
		// These call engine-internal Pak serialization (FPakFile::EncodePakEntriesIntoIndex, FPakInfo)
		// whose signatures DRIFT between UE versions. Default 0 => guaranteed-build core that only
		// provides the byte-level binary delta. Flip to 1 to compile FBinaryMerge, then VERIFY the
		// EncodePakEntriesIntoIndex/FPakInfo signatures against your UE 5.8 engine headers and adapt.
		PublicDefinitions.Add("HDIFFPATCHUE_ENABLE_PAK_REBUILD=0");
	}
}
