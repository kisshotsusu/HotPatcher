// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

// project header
#include "FUnrealPakSettings.h"
#include "FIoStoreSettings.h"
#include "FPatchVersionDiff.h"
#include "FChunkInfo.h"
#include "FReplaceText.h"
#include "ETargetPlatform.h"
#include "FExternFileInfo.h"
#include "FExternDirectoryInfo.h"
#include "FPlatformExternAssets.h"
#include "FPatcherSpecifyAsset.h"
#include "CreatePatch/HotPatcherSettingBase.h"
#include "BinariesPatchFeature.h"
#include "FPlatformBasePak.h"
#include "FPakEncryptionKeys.h"
#include "FBinariesPatchConfig.h"
#include "FHotPatcherVersion.h"
#include "FPakVersion.h"
#include "FPlatformExternAssets.h"
#include "BaseTypes/FCookShaderOptions.h"
#include "BaseTypes/FAssetRegistryOptions.h"
#include "BaseTypes.h"
// engine header
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "FExportPatchSettings.generated.h"


USTRUCT()
struct HOTPATCHERRUNTIME_API FPatherResult
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FAssetDetail> PatcherAssetDetails;
};

USTRUCT(BlueprintType)
struct FCookAdvancedOptions
{
	GENERATED_BODY()
	FCookAdvancedOptions();
	// ConcurrentSave for cooking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="Cook 时是否开启并行序列化保存，可加快大批量资产写入速度"))
	bool bCookParallelSerialize = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="每帧处理的资产数量，用于控制主线程占用与卡顿（仅非并行序列化时生效）"))
	int32 NumberOfAssetsPerFrame = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="按资产类型（UClass）覆写每帧处理的资产数量"))
	TMap<UClass*,int32> OverrideNumberOfAssetsPerFrame;
	FORCEINLINE const TMap<UClass*,int32>& GetOverrideNumberOfAssetsPerFrame()const{ return OverrideNumberOfAssetsPerFrame; }
	UPROPERTY(EditAnywhere, meta=(ToolTip="是否伴随 Cook 为着色器生成配套数据（Accompany Cook），用于着色器编译与缓存"))
	bool bAccompanyCookForShader = false;
};

/** Singleton wrapper to allow for using the setting structure in SSettingsView */
USTRUCT(BlueprintType)
struct HOTPATCHERRUNTIME_API FExportPatchSettings:public FHotPatcherSettingBase
{
	GENERATED_USTRUCT_BODY()
public:

	FExportPatchSettings();
	virtual void Init() override;
	
	FORCEINLINE static FExportPatchSettings* Get()
	{
		static FExportPatchSettings StaticIns;

		return &StaticIns;
	}
	
	FORCEINLINE virtual TArray<FPlatformExternAssets>& GetAddExternAssetsToPlatform()override{return AddExternAssetsToPlatform;}

	FORCEINLINE bool IsAnalysisDiffAssetDependenciesOnly()const {return bAnalysisDiffAssetDependenciesOnly;}
	
	FORCEINLINE FString GetVersionId()const { return VersionId; }
	FString GetBaseVersion()const;
	FORCEINLINE TArray<FString> GetUnrealPakListOptions()const { return GetUnrealPakSettings().UnrealPakListOptions; }
	FORCEINLINE TArray<FReplaceText> GetReplacePakListTexts()const { return ReplacePakListTexts; }
	FORCEINLINE TArray<FString> GetUnrealPakCommandletOptions()const { return GetUnrealPakSettings().UnrealCommandletOptions; }
	FORCEINLINE TArray<ETargetPlatform> GetPakTargetPlatforms()const { return PakTargetPlatforms; }
	TArray<FString> GetPakTargetPlatformNames()const;
	
	FORCEINLINE bool IsSaveDiffAnalysis()const { return IsByBaseVersion() && bStorageDiffAnalysisResults; }
	FORCEINLINE TArray<FString> GetIgnoreDeletionModulesAsset()const{return IgnoreDeletionModulesAsset;}

	// FORCEINLINE bool IsPackageTracker()const { return bPackageTracker; }
	FORCEINLINE bool IsIncludeAssetRegistry()const { return bIncludeAssetRegistry; }
	FORCEINLINE bool IsIncludeGlobalShaderCache()const { return bIncludeGlobalShaderCache; }
	FORCEINLINE bool IsIncludeShaderBytecode()const { return bIncludeShaderBytecode; }
	FORCEINLINE bool IsMakeBinaryConfig()const { return bMakeBinaryConfig; }
	FORCEINLINE bool IsIncludeEngineIni()const { return bIncludeEngineIni; }
	FORCEINLINE bool IsIncludePluginIni()const { return bIncludePluginIni; }
	FORCEINLINE bool IsIncludeProjectIni()const { return bIncludeProjectIni; }

	FORCEINLINE bool IsByBaseVersion()const { return bByBaseVersion; }
	FORCEINLINE bool IsEnableExternFilesDiff()const { return bEnableExternFilesDiff; }
	
	FORCEINLINE bool IsIncludePakVersion()const { return bIncludePakVersionFile; }

	// chunk infomation
	FORCEINLINE bool IsEnableChunk()const { return bEnableChunk; }
	FORCEINLINE TArray<FChunkInfo> GetChunkInfos()const { return ChunkInfos; }

	FORCEINLINE FString GetPakVersionFileMountPoint()const { return PakVersionFileMountPoint; }
	static FPakVersion GetPakVersion(const FHotPatcherVersion& InHotPatcherVersion,const FString& InUtcTime);
	static FString GetSavePakVersionPath(const FString& InSaveAbsPath,const FHotPatcherVersion& InVersion);
	static FString GetPakCommandsSaveToPath(const FString& InSaveAbsPath, const FString& InPlatfornName, const FHotPatcherVersion& InVersion);

	FHotPatcherVersion GetNewPatchVersionInfo();
	bool GetBaseVersionInfo(FHotPatcherVersion& OutBaseVersion)const;
	FString GetCurrentVersionSavePath()const;

	FORCEINLINE bool IsCustomPakNameRegular()const {return bCustomPakNameRegular;}
	FORCEINLINE FString GetPakNameRegular()const { return PakNameRegular;}
	FORCEINLINE bool IsCustomPakPathRegular()const {return bCustomPakPathRegular;}
	FORCEINLINE FString GetPakPathRegular()const { return PakPathRegular;}
	FORCEINLINE bool IsCookPatchAssets()const {return bCookPatchAssets;}
	FORCEINLINE bool IsIgnoreDeletedAssetsInfo()const {return bIgnoreDeletedAssetsInfo;}
	FORCEINLINE bool IsSaveDeletedAssetsToNewReleaseJson()const {return bStorageDeletedAssetsToNewReleaseJson;}
	
	FORCEINLINE FIoStoreSettings GetIoStoreSettings()const { return IoStoreSettings; }
	FORCEINLINE FUnrealPakSettings GetUnrealPakSettings()const {return UnrealPakSettings;}
	FORCEINLINE TArray<FString> GetDefaultPakListOptions()const {return DefaultPakListOptions;}
	FORCEINLINE TArray<FString> GetDefaultCommandletOptions()const {return DefaultCommandletOptions;}

	FORCEINLINE bool IsCreateDefaultChunk()const { return bCreateDefaultChunk; }
	FORCEINLINE bool IsEnableMultiThread()const{ return bEnableMultiThread; }

	FORCEINLINE bool IsStorageNewRelease()const{return bStorageNewRelease;}
	FORCEINLINE bool IsStoragePakFileInfo()const{return bStoragePakFileInfo;}
	// FORCEINLINE bool IsBackupMetadata()const {return bBackupMetadata;}
	FORCEINLINE bool IsEnableProfiling()const { return bEnableProfiling; }
	
	FORCEINLINE FPakEncryptSettings GetEncryptSettings()const{ return EncryptSettings; }
	FORCEINLINE bool IsBinariesPatch()const{ return bBinariesPatch; }
	FORCEINLINE FBinariesPatchConfig GetBinariesPatchConfig()const{ return BinariesPatchConfig; }
	FORCEINLINE bool IsSharedShaderLibrary()const { return GetCookShaderOptions().bSharedShaderLibrary; }
	FORCEINLINE FCookShaderOptions GetCookShaderOptions()const {return CookShaderOptions;}
	FORCEINLINE FAssetRegistryOptions GetSerializeAssetRegistryOptions()const{return SerializeAssetRegistryOptions;}
	FORCEINLINE bool IsImportProjectSettings()const{ return bImportProjectSettings; }

	virtual FString GetCombinedAdditionalCommandletArgs()const override;
	virtual bool IsCookParallelSerialize() const { return CookAdvancedOptions.bCookParallelSerialize; }
public:
	// 是否基于基础版本做差异打包：开启后只打包与基础版本有差异的内容，大幅减小补丁体积
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseVersion", meta=(ToolTip="是否基于基础版本做差异打包。开启后只比对并打包与基础版本不同的内容，显著减小补丁体积"))
		bool bByBaseVersion = false;
	// 基础版本信息文件路径（.json），记录上一个版本的版本号与资源清单，用于差异比对
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "BaseVersion",meta = (RelativeToGameContentDir, EditCondition="bByBaseVersion", ToolTip="基础版本信息文件（.json）路径，记录上一版本的版本号与资源清单，用于差异比对。仅在开启“基于基础版本”时可用"))
		FFilePath BaseVersion;
	// 本次补丁的版本号（如 1.1），会写入发布信息与 pak 版本文件
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "PatchBaseSettings", meta=(ToolTip="本次补丁的版本号（如 1.1），会写入发布信息与 pak 版本文件，用于运行时版本校验"))
		FString VersionId;
	// 是否将 Project Settings 中的配置项一并打入补丁
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Filters", meta=(ToolTip="是否将 Project Settings 中的配置项一并打入补丁（常用于配置热更）"))
		bool bImportProjectSettings = false;

	// require HDiffPatchUE plugin
	// 是否对二进制资源（如 .pak/.utoc）做二进制差异合并，需启用 HDiffPatchUE 插件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch", meta=(ToolTip="是否对二进制资源（如 .pak/.utoc/.ucas）做二进制差异合并。需启用 HDiffPatchUE 插件，否则无法生成补丁"))
		bool bBinariesPatch = false;
	// 二进制差异合并的详细配置（仅在开启二进制补丁时可用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch", meta=(EditCondition="bBinariesPatch", ToolTip="二进制差异合并的详细配置（差分特性、加密、基础版本 pak 列表等）。仅在开启“二进制补丁”时可用"))
		FBinariesPatchConfig BinariesPatchConfig;

	// 只对与基础包有差异的资源进行依赖分析，提高依赖分析的速度
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Filters",meta = (EditCondition = "!bAnalysisFilterDependencies"))
	bool bAnalysisDiffAssetDependenciesOnly = false;
	// allow tracking load asset when cooking
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Filters")
		bool bPackageTracker = true;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooked Files")
		bool bIncludeAssetRegistry = false;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooked Files")
		bool bIncludeGlobalShaderCache = false;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooked Files")
		bool bIncludeShaderBytecode = false;

	// Only in UE5
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ini Config Files")
		bool bMakeBinaryConfig = false;
	// 是否将引擎级 .ini 配置打入补丁
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ini Config Files", meta=(ToolTip="是否将引擎级（Engine）.ini 配置打入补丁"))
		bool bIncludeEngineIni = false;
	// 是否将插件级 .ini 配置打入补丁
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ini Config Files", meta=(ToolTip="是否将插件级（Plugin）.ini 配置打入补丁"))
		bool bIncludePluginIni = false;
	// 是否将项目级 .ini 配置打入补丁
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ini Config Files", meta=(ToolTip="是否将项目级（Project）.ini 配置打入补丁"))
		bool bIncludeProjectIni = false;

	// 是否对外部文件做差异比对（仅打包与基础版本不同的外部文件）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "External Files", meta=(ToolTip="是否对外部文件做差异比对，仅打包与基础版本不同的外部文件，否则全部打入"))
		bool bEnableExternFilesDiff = true;
	// 忽略删除的资源模块列表（差异分析时这些模块的删除不计入）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "External Files", meta=(ToolTip="差异分析时忽略删除的资源模块列表（这些模块的资源删除不计入补丁）"))
		TArray<FString> IgnoreDeletionModulesAsset;
	// 按平台添加打包到补丁里的外部文件/目录
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "External Files", meta=(ToolTip="按目标平台添加需要打入补丁的外部文件与外部目录"))
		TArray<FPlatformExternAssets> AddExternAssetsToPlatform;
	// record patch infomation to pak
	// 是否把补丁版本信息写入 pak（便于运行时校验版本）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "External Files", meta=(ToolTip="是否把补丁版本信息写入 pak，便于运行时校验版本号与校验码"))
		bool bIncludePakVersionFile = false;
	// {
	// 	"versionId": "1.1",
	// 	"baseVersionId": "1.0",
	// 	"date": "2022.01.09-02.52.34",
	// 	"checkCode": "D13EFFEB5716F00CBB823E8E8546FB610531FE37"
	// }
	// pak 内版本信息文件的挂载路径（UFS 虚拟路径）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "External Files",meta=(EditCondition = "bIncludePakVersionFile", ToolTip="pak 内版本信息文件的挂载路径（UFS 虚拟路径，如 ../../../PROJECT/Content/Paks/version.json）。仅在开启“写入 pak 版本信息”时可用"))
		FString PakVersionFileMountPoint;
	// 是否启用 Chunk 分块打包（把资源拆分到多个 pak）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Options", meta=(ToolTip="是否启用 Chunk 分块打包，把资源拆分到多个 pak，便于按需下载/挂载"))
		bool bEnableChunk = false;

	// If the resource is not contained by any chunk, create a default chunk storage
	// 未被任何 Chunk 收纳的资源是否自动归入默认 Chunk
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Options", meta = (EditCondition = "bEnableChunk", ToolTip="未被任何 Chunk 收纳的资源是否自动归入默认 Chunk 存储。仅在开启 Chunk 时可用"))
		bool bCreateDefaultChunk = false;
	// Chunk 配置列表（每块包含哪些资源/平台）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Options", meta = (EditCondition = "bEnableChunk", ToolTip="Chunk 配置列表，定义每块包含哪些资源/平台。仅在开启 Chunk 时可用"))
		TArray<FChunkInfo> ChunkInfos;

	/*
	 * Cook Asset in current patch
	 * shader code gets saved inline inside material assets
	 * bShareMaterialShaderCode as false
	 */
	// 是否在本次补丁内重新 Cook 资源（着色器内联到材质资产）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="是否在本次补丁内重新 Cook 资源（着色器代码内联进材质资产，bShareMaterialShaderCode 视为 false）"))
		bool bCookPatchAssets = true;
	// Cook 高级选项（并行序列化/每帧资产数等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(EditCondition = "bCookPatchAssets", ToolTip="Cook 高级选项：并行序列化、每帧资产数等。仅在开启“本次补丁内 Cook”时可用"))
		FCookAdvancedOptions CookAdvancedOptions;
	// 着色器库相关 Cook 选项（是否共享着色器库）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(EditCondition = "bCookPatchAssets", ToolTip="着色器库相关 Cook 选项（是否生成/共享着色器库）。仅在开启“本次补丁内 Cook”时可用"))
		FCookShaderOptions CookShaderOptions;
	// 资源注册表序列化选项
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(EditCondition = "bCookPatchAssets", ToolTip="资源注册表（AssetRegistry）序列化选项。仅在开启“本次补丁内 Cook”时可用"))
		FAssetRegistryOptions SerializeAssetRegistryOptions;
	// support UE4.26 later
	// IoStore 设置（UE4.26+ 的 .utoc/.ucas 容器）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(EditCondition = "!bCookPatchAssets", ToolTip="IoStore 设置（UE4.26+ 的 .utoc/.ucas 容器）。仅在关闭“本次补丁内 Cook”时可配"))
		FIoStoreSettings IoStoreSettings;
	// UnrealPak 命令行设置（加密/压缩等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="UnrealPak 命令行设置：压缩格式、加密索引等"))
		FUnrealPakSettings UnrealPakSettings;

	// using in Pak and IO Store
	// 传给 UnrealPak 的默认 pak 列表选项
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="传给 UnrealPak 的默认 pak 列表选项（如压缩/排序指令）"))
		TArray<FString> DefaultPakListOptions;
	// 传给 Cook Commandlet 的默认参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="传给 Cook Commandlet 的默认参数"))
		TArray<FString> DefaultCommandletOptions;

	
	// pak 加密设置（AES 密钥/加密索引等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="pak 加密设置：AES 密钥、是否加密索引与内容等"))
		FPakEncryptSettings EncryptSettings;

	// 打包时对 pak 列表文本做替换的规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="打包时对 pak 列表文本做替换的规则（如路径重定向）"))
		TArray<FReplaceText> ReplacePakListTexts;
	// 本次补丁要打包的目标平台列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="本次补丁要打包的目标平台列表（如 Windows、Android、IOS）"))
		TArray<ETargetPlatform> PakTargetPlatforms;
	// 是否自定义 pak 命名规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="是否自定义 pak 文件命名规则（关闭则使用默认命名）"))
		bool bCustomPakNameRegular = false;
	// Can use value: {VERSION} {BASEVERSION} {CHUNKNAME} {PLATFORM} 
	// pak 文件名规则，可用宏 {VERSION} {BASEVERSION} {CHUNKNAME} {PLATFORM}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options",meta=(EditCondition = "bCustomPakNameRegular", ToolTip="pak 文件名规则，可用宏：{VERSION} {BASEVERSION} {CHUNKNAME} {PLATFORM}。仅在开启“自定义命名规则”时可用"))
		FString PakNameRegular = TEXT("{VERSION}_{CHUNKNAME}_{PLATFORM}_001_P");
	// 是否自定义 pak 输出路径规则
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options", meta=(ToolTip="是否自定义 pak 输出路径规则（关闭则使用默认路径）"))
		bool bCustomPakPathRegular = false;
	// Can use value: {VERSION} {BASEVERSION} {CHUNKNAME} {PLATFORM} 
	// pak 输出路径规则，可用宏 {CHUNKNAME} {PLATFORM}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pak Options",meta=(EditCondition = "bCustomPakPathRegular", ToolTip="pak 输出路径规则，可用宏：{CHUNKNAME} {PLATFORM}。仅在开启“自定义路径规则”时可用"))
		FString PakPathRegular = TEXT("{CHUNKNAME}/{PLATFORM}");

	// 是否保存本次补丁的发布信息（release json）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveTo", meta=(ToolTip="是否保存本次补丁的发布信息（release json），包含版本号/资源清单/哈希"))
		bool bStorageNewRelease = true;
	// 是否保存 pak 文件信息（大小/哈希等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveTo", meta=(ToolTip="是否保存 pak 文件信息（文件大小、哈希等），供分发与校验使用"))
		bool bStoragePakFileInfo = true;
	// dont display deleted asset info in patcher
	// 是否在补丁信息中隐藏被删除的资源
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveTo", meta=(ToolTip="是否在补丁信息中隐藏被删除的资源（仅影响信息展示，不阻止删除生效）"))
		bool bIgnoreDeletedAssetsInfo = false;
	// 是否把被删除资源信息写入新发布的 release json
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveTo", meta=(ToolTip="是否把被删除资源信息写入新发布的 release json（供客户端感知资源删除）"))
		bool bStorageDeletedAssetsToNewReleaseJson = true;
	// 是否保存差异分析结果（仅在基于基础版本时可用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveTo",meta=(EditCondition="bByBaseVersion", ToolTip="是否保存差异分析结果（资源增删改明细）。仅在开启“基于基础版本”时可用"))
		bool bStorageDiffAnalysisResults = true;
	// 是否保存 UnrealPak 列表文件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveTo", meta=(ToolTip="是否保存 UnrealPak 列表文件（.txt），记录本次打包进 pak 的全部文件"))
		bool bStorageUnrealPakList = true;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveTo")
	// 	bool bStorageAssetDependencies = false;
	// UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "SaveTo")
	// 	bool bBackupMetadata = false;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
		bool bEnableMultiThread = false;

	// 是否启用打包性能分析
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Advanced", meta=(ToolTip="是否启用打包性能分析，输出各阶段耗时，便于定位打包瓶颈"))
		bool bEnableProfiling = false;
	// Cook 产物（.uasset 等）的暂存目录
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Advanced", meta=(ToolTip="Cook 产物（.uasset 等）的暂存目录，默认 [PROJECTDIR]/Saved/Cooked"))
		FString StorageCookedDir = TEXT("[PROJECTDIR]/Saved/Cooked");

	FString GetStorageCookedDir()const;
	FString GetChunkSavedDir(const FString& InVersionId,const FString& InBaseVersionId,const FString& InChunkName,const FString& InPlatformName)const;

};

struct HOTPATCHERRUNTIME_API FReplacePakRegular
{
	FReplacePakRegular()=default;
	FReplacePakRegular(const FString& InVersionId,const FString& InBaseVersionId,const FString& InChunkName,const FString& InPlatformName):
		VersionId(InVersionId),BaseVersionId(InBaseVersionId),ChunkName(InChunkName),PlatformName(InPlatformName){}
	FString VersionId;
	FString BaseVersionId;
	FString ChunkName;
	FString PlatformName;
};
