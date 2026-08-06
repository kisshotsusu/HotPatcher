#pragma once
// project
#include "ETargetPlatform.h"

// engine header
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "FIoStoreSettings.generated.h"

// -Output=E:\UnrealProjects\StarterContent\Package\DLC2\WindowsNoEditor\StarterContent\Content\Paks\StarterContent-WindowsNoEditor_0_P.utoc
// -ContainerName=StarterContent
// -PatchSource=E:\UnrealProjects\StarterContent\Releases\1.0\WindowsNoEditor\StarterContent-WindowsNoEditor*.utoc
// -GenerateDiffPatch
// -ResponseFile="C:\Users\visionsmile\AppData\Roaming\Unreal Engine\AutomationTool\Logs\E+UnrealEngine+Launcher+UE_4.26\PakListIoStore_StarterContent.txt"

USTRUCT(BlueprintType)
struct FIoStorePlatformContainers
{
	GENERATED_USTRUCT_BODY()
	// Saved/StagedBuilds/Windows
	// Saved/StagedBuilds/Android_ASTC
	// │  Manifest_NonUFSFiles_Win64.txt
	// │  ThirdPerson_UE5.exe
	// ├─Engine
	// └─ThirdPerson_UE5
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 基础包目录：指向游戏打包后的根目录（包含 CodeBuild/Content/Paks 的那一层）。
	// 例如 Windows 平台为：Saved/StagedBuilds/Windows
	FDirectoryPath BasePackageStagedRootDir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 是否生成增量容器（Diff Patch）：开启后需要 PatchSourceOverride 指向基础包的 .utoc
	bool bGenerateDiffPatch = false;
	
	// global.utoc file
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// 补丁自己的 global.utoc 输出路径；留空时自动输出到补丁目录（不会覆盖基础包）
	FFilePath GlobalContainersOverride;
	// -PatchSource=E:\UnrealProjects\StarterContent\Releases\1.0\WindowsNoEditor\StarterContent-WindowsNoEditor*.utoc -GenerateDiffPatch
	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(EditCondition="bGenerateDiffPatch"))
	// 基础包容器通配符，例如 D:/xxx/Saved/StagedBuilds/Windows/CodeBuild/Content/Paks/*.utoc
	FFilePath PatchSourceOverride;
};

USTRUCT(BlueprintType)
struct FIoStoreSettings
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
		// 【IoStore 补丁开关】勾选后，任务会额外生成 .utoc/.ucas 容器补丁。
		// 适用于游戏打包设置里开启了 Use Io Store（UE5 默认）的情况；
		// 生成的文件为：补丁名.pak + 补丁名.utoc + 补丁名.ucas，一起放入游戏 Content/Paks 目录即可。
		// 不勾选时保持原来的普通 .pak 补丁流程。
		bool bIoStore = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
		// 是否把 .ubulk/.uptnl 等 Bulk 数据也放进 IoStore 容器。
		// 游戏使用 IoStore 时建议勾选；不勾选时 Bulk 数据会留在普通 .pak 里，
		// 可能导致带 Bulk 数据的资源（如大型贴图/网格）在 IoStore 客户端中读取不到。
		bool bAllowBulkDataInIoStore = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		// 追加到 IoStore 响应文件的额外参数（高级选项，一般留空）
		TArray<FString> IoStorePakListOptions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		// 追加到 IoStore 命令let 的额外参数（高级选项，一般留空）
		TArray<FString> IoStoreCommandletOptions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		// 各平台的基础包配置：Windows 平台键名为 Windows。
		// 不配置时自动使用 Saved/StagedBuilds/<平台名> 作为基础包目录。
		TMap<ETargetPlatform,FIoStorePlatformContainers> PlatformContainers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		// 是否保存 IoStore 的 Pak 列表文件（调试用）
		bool bStoragePakList = true;
	// Metadata/BulkDataInfo.ubulkmanifest
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bStorageBulkDataInfo = true;
};
