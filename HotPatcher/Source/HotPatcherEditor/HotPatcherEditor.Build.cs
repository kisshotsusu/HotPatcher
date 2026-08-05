// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;

public class HotPatcherEditor : ModuleRules
{
	public HotPatcherEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicIncludePaths.AddRange(new string[] { });
		PrivateIncludePaths.AddRange(new string[] {});
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"UMG",
				"UMGEditor",
				"Core",
				"Json",
				"ContentBrowser",
				"SandboxFile",
				"JsonUtilities",
				"TargetPlatform",
				"DesktopPlatform",
				"Projects",
				"Settings",
				"EditorStyle",
				"HTTP",
				"RHI",
				"EngineSettings",
				"AssetRegistry",
				"PakFileUtilities",
				"HotPatcherRuntime",
				"BinariesPatchFeature",
				"HotPatcherCore",
				"ToolMenus",
				"TraceLog",
				"IoStoreUtilities"
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UnrealEd",
				"Projects",
				"DesktopPlatform",
				"InputCore",
				"LevelEditor",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"RenderCore"
			}
		);

		PublicDefinitions.Add("ENABLE_COOK_ENGINE_MAP=0");
		PublicDefinitions.Add("ENABLE_COOK_PLUGIN_MAP=0");
	}
}
