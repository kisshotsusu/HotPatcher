#pragma once
// project header
#include "FPatcherSpecifyAsset.h"
#include "FExternFileInfo.h"
#include "ETargetPlatform.h"
#include "FPlatformExternFiles.h"
#include "FPlatformExternAssets.h"

// engine header
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "FAssetScanConfig.generated.h"

USTRUCT(BlueprintType)
struct HOTPATCHERRUNTIME_API FAssetScanConfig
{
	GENERATED_USTRUCT_BODY()

public:
	// 是否开启资源加载追踪（Cook 时记录被加载的资源，用于更完整的依赖分析）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="是否开启资源加载追踪。开启后 Cook 阶段会记录实际被加载的资源，使依赖分析更完整"))
	bool bPackageTracker = true;
	
    // 需要扫描/打包的资产目录（相对 Content 目录）
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (RelativeToGameContentDir, LongPackageName, ToolTip="需要纳入扫描/打包的资产目录（相对 Content 目录）。只有这些目录下的资产会被打进补丁"))
    TArray<FDirectoryPath> AssetIncludeFilters;
    // Ignore directories in AssetIncludeFilters 
    // 在“包含目录”中进一步排除的子目录
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (RelativeToGameContentDir, LongPackageName, ToolTip="在“包含目录”基础上进一步排除的子目录"))
    TArray<FDirectoryPath> AssetIgnoreFilters;
    
    // 仅打包被其它资源引用的资产（剔除未被引用的孤立资产）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="仅打包被其它资源引用的资产，剔除未被任何资源引用的孤立资产"))
    bool bIncludeHasRefAssetsOnly = false;
    // 是否对过滤出的资产做依赖分析（收集其引用链，确保被依赖资源一起打入）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="是否对过滤出的资产做依赖分析，自动收集其引用链，确保被依赖的资源一起打入补丁"))
    bool bAnalysisFilterDependencies=true;
	
    // 依赖分析时要追踪的依赖类型（如硬引用/软引用/脚本/搜索等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="依赖分析时要追踪的依赖类型（硬引用、软引用、搜索引用、脚本引用等）"))
    TArray<EAssetRegistryDependencyTypeEx> AssetRegistryDependencyTypes;
    // 显式指定要打入的单个资产（及其附加选项，如 Cook 平台、是否分析依赖）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="显式指定要打入的单个资产，可配置其 Cook 平台与是否做依赖分析等附加选项"))
    TArray<FPatcherSpecifyAsset> IncludeSpecifyAssets;
    // 是否递归分析 Widget 树的依赖（UMG 控件引用的资源）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="是否递归分析 Widget（UMG）树的依赖，确保控件引用的纹理/字体等资源一并打入"))
    bool bRecursiveWidgetTree = true;
	// 是否分析材质实例对母材质的依赖
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="是否分析材质实例对母材质的依赖，确保实例引用的母材质一并打入"))
	bool bAnalysisMaterialInstance = true;
	// 是否支持世界组合（World Composition）分割的关卡流送
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="是否支持 World Composition 拆分的世界（关卡流送），将其子关卡一并纳入扫描"))
	bool bSupportWorldComposition = false;
	
    // 是否强制跳过部分内容（编辑器内容、指定资产/类/目录等）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="是否强制跳过部分内容（如编辑器内容、指定目录/资产/类），减小补丁体积"))
    bool bForceSkipContent = true;
    // force exclude asset folder e.g. Exclude editor content when cooking in Project Settings
    // 强制排除的资产目录（如 Project Settings 中 Cook 时排除的编辑器内容）
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (RelativeToGameContentDir, LongPackageName, EditCondition="bForceSkipContent", ToolTip="强制排除的资产目录（例如 Project Settings 中 Cook 时排除的编辑器内容）。仅在开启“强制跳过内容”时可用"))
    TArray<FDirectoryPath> ForceSkipContentRules;
    // 强制排除的单个资产
    UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (EditCondition="bForceSkipContent", ToolTip="强制排除的单个资产（按软对象路径指定）。仅在开启“强制跳过内容”时可用"))
    TArray<FSoftObjectPath> ForceSkipAssets;
    // 强制排除的资产类（该类下的所有资产都不打入）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition="bForceSkipContent", ToolTip="强制排除的资产类（该类下的所有资产都不打入补丁）。仅在开启“强制跳过内容”时可用"))
    TArray<UClass*> ForceSkipClasses;

	bool IsMatchForceSkip(const FSoftObjectPath& ObjectPath,FString& OutReason);

};