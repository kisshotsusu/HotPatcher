#pragma once
#include "ETargetPlatform.h"
#include "FPlatformBasePak.h"
#include "BinariesPatchFeature.h"
#include "FPakEncryptionKeys.h"

// engine header
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "HAL/FileManager.h"
#include "FBinariesPatchConfig.generated.h"

struct FPakCommandItem
{
	FString AssetAbsPath;
	FString AssetMountPath;
};
// 文件匹配规则：决定某文件“命中规则”时是纳入补丁还是被排除
UENUM(BlueprintType, meta=(ToolTip="文件匹配规则：MATCH=满足下方条件才参与二进制补丁；IGNORE=满足条件则从补丁中排除。"))
enum class EMatchRule : uint8
{
	None,    // 不处理
	MATCH,   // 符合条件 -> 参与补丁
	IGNORE   // 符合条件 -> 忽略（不参与补丁）
};

// 文件大小比较运算符
UENUM(BlueprintType, meta=(ToolTip="按文件大小比较的运算符：大于/小于/等于；None 表示不按大小过滤。"))
enum class EMatchOperator : uint8
{
	None,        // 不参与大小比较
	GREAT_THAN,  // 大于阈值
	LESS_THAN,   // 小于阈值
	EQUAL        // 等于阈值
};

// 单条匹配规则：按扩展名/资源类型 + 大小，筛选参与二进制差分补丁的文件
USTRUCT(BlueprintType, meta=(ToolTip="单条文件匹配规则（按扩展名/资源类型与大小筛选参与二进制补丁的文件）。"))
struct FMatchRule
{
	GENERATED_BODY()

	// 命中规则时是“纳入”还是“忽略”
	UPROPERTY(EditAnywhere, meta=(ToolTip="匹配或忽略：MATCH=满足条件才参与补丁；IGNORE=满足条件则排除。"))
	EMatchRule Rule = EMatchRule::None;

	// 大小比较运算符
	UPROPERTY(EditAnywhere, meta=(ToolTip="大小比较运算符：大于/小于/等于（None=不按大小过滤）。"))
	EMatchOperator Operator = EMatchOperator::None;

	// 阈值大小，单位 KiB
	UPROPERTY(EditAnywhere, meta=(EditCondition="Operator!=EMatchOperator::None", ToolTip="阈值大小，单位 KiB。仅当 Operator 不为 None 时生效。"))
	float Size = 100;

	// 扩展名过滤，例如 .ini/.lua；为空表示匹配所有格式
	UPROPERTY(EditAnywhere, meta=(ToolTip="按扩展名过滤，例如 .ini/.lua；为空表示匹配所有格式。"))
	TArray<FString> Formaters;

	// 资源类型过滤；为空表示匹配所有类型
	UPROPERTY(EditAnywhere, meta=(ToolTip="按资源类型过滤；为空表示匹配所有类型。"))
	TArray<FString> AssetTypes;
};

// 二进制差分补丁配置：在 HotPatcher 的 Patch 设置中暴露给编辑器，
// 控制如何基于“旧基础版本”为资源文件（.pak/.utoc/.ucas 等）生成二进制补丁。
USTRUCT(BlueprintType, meta=(ToolTip="二进制差分补丁配置：基于旧基础版本为资源文件（.pak/.utoc/.ucas 等）生成二进制补丁。"))
struct HOTPATCHERRUNTIME_API FBinariesPatchConfig
{
	GENERATED_BODY();
public:
	FORCEINLINE FPakEncryptSettings GetEncryptSettings()const{ return EncryptSettings; }
	
	// 选择用于生成二进制补丁的差分特性；下拉项来自已注册的 ModularFeatures 实现（如 HDiffPatchUE）。选 None 则不做二进制差分。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch", meta=(ToolTip="选择用于生成二进制补丁的差分特性（下拉项来自已注册的 ModularFeatures 实现，例如 HDiffPatchUE）。选 None 则不做二进制差分。"))
	EBinariesPatchFeature BinariesPatchType = EBinariesPatchFeature::None;

	// 旧版本 Cook 输出目录（基础版本源文件所在路径，用于差分比对）。
	// 注意：当前未暴露为可编辑 UPROPERTY，仅作为内部字段由代码赋值。
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch")
	FDirectoryPath OldCookedDir;

	// 二进制补丁产物（及基础包）的加密设置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch", meta=(ToolTip="二进制补丁产物（及基础包）的加密设置。"))
	FPakEncryptSettings EncryptSettings;

	// 基础版本 pak 列表（按平台），差分将基于这些基础包生成
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch", meta=(ToolTip="基础版本 pak 列表（按平台），二进制差分将基于这些基础包生成。"))
	TArray<FPlatformBasePak> BaseVersionPaks;

	// 文件匹配/忽略规则：决定哪些文件参与二进制差分补丁
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch", meta=(ToolTip="文件匹配/忽略规则：决定哪些文件参与二进制差分补丁。"))
	TArray<FMatchRule> MatchRules;
	// etc .ini/.lua
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BinariesPatch")
	// TArray<FString> IgnoreFileRules;
	bool IsMatchIgnoreRules(const FPakCommandItem& File);

	// FORCEINLINE TArray<FString> GetBinariesPatchIgnoreFileRules()const {return IgnoreFileRules;}
	FORCEINLINE TArray<FMatchRule> GetMatchRules()const{ return MatchRules; }
	FORCEINLINE TArray<FPlatformBasePak> GetBaseVersionPaks()const {return BaseVersionPaks;};
	FString GetBinariesPatchFeatureName() const;
	FString GetOldCookedDir() const;
	FString GetBasePakExtractCryptoJson() const;
	TArray<FString> GetBaseVersionPakByPlatform(ETargetPlatform Platform);
	
};
