#pragma once
#include "FPlatformExternFiles.h"
#include "FPatcherSpecifyAsset.h"
#include "FPlatformExternAssets.h"
#include "BaseTypes/AssetManager/FAssetDependenciesInfo.h"
// engine
#include "CoreMinimal.h"
#include "FAssetScanConfig.h"
#include "Engine/EngineTypes.h"
#include "HotPatcherSettingBase.generated.h"

USTRUCT(BlueprintType)
struct HOTPATCHERRUNTIME_API FPatcherEntitySettingBase
{
    GENERATED_BODY();
    virtual ~FPatcherEntitySettingBase(){}
};


USTRUCT(BlueprintType)
struct HOTPATCHERRUNTIME_API FHotPatcherSettingBase:public FPatcherEntitySettingBase
{
    GENERATED_USTRUCT_BODY()
    FHotPatcherSettingBase();
    
    virtual TArray<FPlatformExternAssets>& GetAddExternAssetsToPlatform();
    virtual void Init();

    virtual TArray<FExternFileInfo> GetAllExternFilesByPlatform(ETargetPlatform InTargetPlatform,bool InGeneratedHash = false);
    virtual TMap<ETargetPlatform,FPlatformExternFiles> GetAllPlatfotmExternFiles(bool InGeneratedHash = false);
    virtual TArray<FExternFileInfo> GetAddExternFilesByPlatform(ETargetPlatform InTargetPlatform,bool InGeneratedHash);
    virtual TArray<FExternDirectoryInfo> GetAddExternDirectoryByPlatform(ETargetPlatform InTargetPlatform);

    virtual FString GetSaveAbsPath()const;
    FORCEINLINE_DEBUGGABLE virtual FString GetSavePath()const{ return SavePath.Path; }
    
    FORCEINLINE virtual bool IsStandaloneMode()const {return bStandaloneMode;}
    FORCEINLINE virtual bool IsSaveConfig()const {return bStorageConfig;}
    FORCEINLINE virtual TArray<FString> GetAdditionalCommandletArgs()const{return AdditionalCommandletArgs;}
    virtual FString GetCombinedAdditionalCommandletArgs()const;

    FORCEINLINE virtual bool IsForceSkipContent()const{return GetAssetScanConfig().bForceSkipContent;}
    FORCEINLINE virtual TArray<FDirectoryPath> GetForceSkipContentRules()const {return GetAssetScanConfig().ForceSkipContentRules;}
    FORCEINLINE virtual TArray<FSoftObjectPath> GetForceSkipAssets()const {return GetAssetScanConfig().ForceSkipAssets;}
    virtual TArray<FString> GetAllSkipContents()const;

    FORCEINLINE virtual TArray<FDirectoryPath>& GetAssetIncludeFilters() { return GetAssetScanConfigRef().AssetIncludeFilters; }
    FORCEINLINE virtual TArray<FPatcherSpecifyAsset>& GetIncludeSpecifyAssets() { return GetAssetScanConfigRef().IncludeSpecifyAssets; }
    FORCEINLINE virtual TArray<FDirectoryPath>& GetAssetIgnoreFilters()  { return GetAssetScanConfigRef().AssetIgnoreFilters; }
    FORCEINLINE TArray<FPatcherSpecifyAsset> GetSpecifyAssets()const { return GetAssetScanConfig().IncludeSpecifyAssets; }
    FORCEINLINE bool AddSpecifyAsset(FPatcherSpecifyAsset const& InAsset)
    {
        return GetAssetScanConfigRef().IncludeSpecifyAssets.AddUnique(InAsset) != INDEX_NONE;
    }
    FORCEINLINE virtual TArray<UClass*>& GetForceSkipClasses() { return GetAssetScanConfigRef().ForceSkipClasses; }
    // virtual TArray<FString> GetAssetIgnoreFiltersPaths()const;
    FORCEINLINE bool IsAnalysisFilterDependencies()const { return GetAssetScanConfig().bAnalysisFilterDependencies; }
    FORCEINLINE bool IsRecursiveWidgetTree()const {return GetAssetScanConfig().bRecursiveWidgetTree;}
    FORCEINLINE bool IsAnalysisMatInstance()const { return GetAssetScanConfig().bAnalysisMaterialInstance; }
    FORCEINLINE bool IsIncludeHasRefAssetsOnly()const { return GetAssetScanConfig().bIncludeHasRefAssetsOnly; }
    FORCEINLINE TArray<EAssetRegistryDependencyTypeEx> GetAssetRegistryDependencyTypes()const { return GetAssetScanConfig().AssetRegistryDependencyTypes; }
    FORCEINLINE bool IsPackageTracker()const { return GetAssetScanConfig().bPackageTracker; }

    FORCEINLINE FAssetScanConfig GetAssetScanConfig()const{ return AssetScanConfig; }
    FORCEINLINE FAssetScanConfig& GetAssetScanConfigRef() { return AssetScanConfig; }
    FORCEINLINE EHashCalculator GetHashCalculator()const { return HashCalculator; }
    virtual ~FHotPatcherSettingBase(){}
    
public:
    // 资产扫描配置：包含/排除目录、依赖分析、强制跳过等内容过滤规则
    UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Asset Filters", meta=(ToolTip="资产扫描配置：包含/排除目录、依赖分析、强制跳过等内容过滤规则"))
    FAssetScanConfig AssetScanConfig;

    // backup current project Cooked/PLATFORM/PROJECTNAME/Metadata directory
    // 是否把当前工程的 Cooked/PLATFORM/PROJECTNAME/Metadata 目录作为配置备份保存
    UPROPERTY(EditAnywhere, Category = "SaveTo", meta=(ToolTip="是否把当前工程 Cooked/PLATFORM/PROJECTNAME/Metadata 目录作为配置备份一并保存"))
    bool bStorageConfig = true;
    // 补丁与各类产物（pak/json/列表文件）的输出目录
    UPROPERTY(EditAnywhere, Category = "SaveTo", meta=(ToolTip="补丁与各类产物（pak/json/列表文件）的输出保存目录"))
    FDirectoryPath SavePath;
    
    // 资源哈希算法（用于差异比对与完整性校验），默认 MD5
    UPROPERTY(EditAnywhere, Category = "Advanced", meta=(ToolTip="资源内容哈希算法（用于差异比对与完整性校验），默认 MD5"))
    EHashCalculator HashCalculator = EHashCalculator::MD5;
    
    // create a UE4Editor-cmd.exe process execute patch mission.
    // 是否以独立进程（UE4Editor-cmd.exe）执行打包任务，避免阻塞编辑器
    UPROPERTY(EditAnywhere, Category = "Advanced", meta=(ToolTip="是否以独立进程（UE4Editor-cmd.exe / UnrealEditor-cmd）执行打包任务，避免阻塞编辑器主线程"))
    bool bStandaloneMode = true;
    // 追加传给 Cook/UnrealPak 命令行的额外参数
    UPROPERTY(EditAnywhere, Category = "Advanced", meta=(ToolTip="追加传给 Cook / UnrealPak 命令行的额外参数"))
    TArray<FString> AdditionalCommandletArgs;
    
};