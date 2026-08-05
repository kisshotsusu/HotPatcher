// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.Collections.Generic;
using System.IO;

public class HotPatcherCore : ModuleRules
{
	public HotPatcherCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory,"Public/CommandletBase"),
				Path.Combine(EngineDirectory,"Source/Runtime/CoreUObject/Internal/Serialization")
			}
			);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"UMG",
				"UMGEditor",
				"Core",
				"Json",
				"LevelSequence",
				"ContentBrowser",
				"SandboxFile",
				"JsonUtilities",
				"TargetPlatform",
				"DesktopPlatform",
				"Projects",
				"Settings",
				"HTTP",
				"RHI",
				"EngineSettings",
				"AssetRegistry",
				"PakFileUtilities",
				"HotPatcherRuntime",
				"BinariesPatchFeature",
				"TraceLog",
				"DeveloperToolSettings",
				"IoStoreUtilities"
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"DesktopPlatform",
				"InputCore",
				"CoreUObject",
				"Engine",
				"Sockets",
				"DerivedDataCache",
				"RenderCore"
			}
		);

		switch (Target.Configuration)
		{
			case UnrealTargetConfiguration.Debug:
			{
				PublicDefinitions.Add("COMPILER_CONFIGURATION_NAME=\"Debug\"");
				break;
			}
			case UnrealTargetConfiguration.DebugGame:
			{
				PublicDefinitions.Add("COMPILER_CONFIGURATION_NAME=\"DebugGame\"");
				break;
			}
			case UnrealTargetConfiguration.Development:
			{
				PublicDefinitions.Add("COMPILER_CONFIGURATION_NAME=\"Development\"");
				break;
			}
			default:
			{
				PublicDefinitions.Add("COMPILER_CONFIGURATION_NAME=\"\"");
				break;
			}
		};
		
		PublicDefinitions.Add("ENABLE_COOK_LOG=1");
		PublicDefinitions.Add("ENABLE_COOK_ENGINE_MAP=0");
		PublicDefinitions.Add("ENABLE_COOK_PLUGIN_MAP=0");
		PublicDefinitions.AddRange(new string[]
		{
			"TOOL_NAME=\"HotPatcher\"",
			"CURRENT_VERSION_ID=82",
			"CURRENT_PATCH_ID=0",
			"REMOTE_VERSION_FILE=\"https://imzlp.com/opensource/version.json\""
		});
	}
}
