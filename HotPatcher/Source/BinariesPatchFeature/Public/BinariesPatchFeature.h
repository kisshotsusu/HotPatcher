// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Features/IModularFeature.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Misc/EnumRange.h"
#include "BinariesPatchFeature.generated.h"

#define BINARIES_DIFF_PATCH_FEATURE_NAME TEXT("BinariesDiffPatchFeatures")

// 二进制差分补丁特性枚举。下拉项由各 ModularFeatures 实现在注册时动态追加
// （例如 HDiffPatchUE），编辑器里的“Binaries Patch Type”即取自此处。
UENUM(BlueprintType, meta=(ToolTip="二进制差分补丁特性类型。下拉项由各 ModularFeatures 实现在注册时动态追加（例如 HDiffPatchUE）；选 None 表示不做二进制差分。"))
enum class EBinariesPatchFeature : uint8
{
	None,            // 不做二进制差分
	Count UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EBinariesPatchFeature, EBinariesPatchFeature::Count);

// 二进制差分补丁特性接口：由具体实现（如 HDiffPatchUE）注册到 IModularFeatures，
// HotPatcher 编辑器侧（GenerateBinariesPatch）调用 CreateDiff 产出 .patch，
// 运行时客户端（如 CloudUpdate）调用 PatchDiff/PatchDiffToFile 重建新文件。
struct BINARIESPATCHFEATURE_API IBinariesDiffPatchFeature : public IModularFeature
{
	virtual ~IBinariesDiffPatchFeature(){};

	// 编辑器侧：用 New 与 Old 两份整文件数据生成二进制补丁 OutPatch（写入 .patch）。
	virtual bool CreateDiff(const TArray<uint8>& NewData, const TArray<uint8>& OldData, TArray<uint8>& OutPatch) = 0;

	// 运行时：用 Old 基础文件 + Patch 补丁重建 New 文件（整文件缓冲，受 ~2 GiB 内存上限限制）。
	virtual bool PatchDiff(const TArray<uint8>& OldData, const TArray<uint8>& PatchData, TArray<uint8>& OutNewData) = 0;

	// 返回本特性的名称（如 "HDiffPatchUE"），需与 FBinariesPatchConfig::BinariesPatchType 选择值一致。
	virtual FString GetFeatureName()const = 0;

	// 基于文件的 apply（从磁盘重建新文件）。
	// 默认实现先把文件读入内存再调 PatchDiff（与旧行为一致，仍受 ~2 GiB 上限限制）；
	// 支持流式的能力（如 HDiffPatchUE）重写此函数，改为按固定大小分块直接从磁盘读写，
	// 解除大文件的内存上限。声明为非纯虚，保证已有实现无需改动即可编译。
	virtual bool PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath);
};

class FBinariesPatchFeatureModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
