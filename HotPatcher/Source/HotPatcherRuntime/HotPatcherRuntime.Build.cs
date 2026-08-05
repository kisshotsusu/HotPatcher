// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class HotPatcherRuntime : ModuleRules
{
	public HotPatcherRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(EngineDirectory,"Source/Runtime/Launch"),
				Path.Combine(ModuleDirectory,"Public"),
				Path.Combine(ModuleDirectory,"Public/BaseTypes"),
				Path.Combine(ModuleDirectory,"Public/Templates")
			}
			);

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"TargetPlatform"
			});
		}
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"RHI",
				"Core",
				"Projects",
				"Json",
				"JsonUtilities",
				"PakFile",
				"AssetRegistry",
				"BinariesPatchFeature"
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"HTTP",
				"Sockets",
				"RenderCore"
			}
		);
		
		bLegacyPublicIncludePaths = false;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}
