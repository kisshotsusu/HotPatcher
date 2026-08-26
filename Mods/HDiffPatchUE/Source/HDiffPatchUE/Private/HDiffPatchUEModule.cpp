// Copyright (c) rebuilt per imzlp article 25136.
#include "Modules/ModuleManager.h"
#include "Features/IModularFeatures.h"
#include "Logging/LogMacros.h" // DEFINE_LOG_CATEGORY_STATIC
#include "HDiffPatchFeatureImpl.h"

DEFINE_LOG_CATEGORY_STATIC(LogHDiffPatchUE, Log, All);

// 注意：IBinariesDiffPatchFeature::PatchDiffToFile 的默认实现已在 BinariesPatchFeature 模块
// 中定义，并通过 BINARIESPATCHFEATURE_API 导出（HDiffPatchUE 已将其列为公共依赖）。
// 因此此处【不要】再重复定义该接口函数，否则单体打包时两份定义会触发 LNK2005/LNK1169。
// 本模块实际使用 FHDiffPatchFeature（见 HDiffPatchFeatureImpl.cpp）的流式重写版本。

class FHDiffPatchUEModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		Feature = MakeShared<FHDiffPatchFeature>();
		// 以与 HotPatcher 的 GenerateBinariesPatch 查询相同的 Modular Feature 名注册本实现。
		IModularFeatures::Get().RegisterModularFeature(BINARIES_DIFF_PATCH_FEATURE_NAME, Feature.Get());
	#if HDIFFPATCHUE_ENABLE_PAK_REBUILD
		const TCHAR* PakRebuildStatus = TEXT("enabled");
	#else
		const TCHAR* PakRebuildStatus = TEXT("disabled");
	#endif
		UE_LOG(LogHDiffPatchUE, Log,
			TEXT("HDiffPatchUE (self-contained binary delta) registered as '%s'. "
			     "Byte-level CreateDiff/PatchDiff is available. Pak rebuild (FBinaryMerge) is %s."),
			*Feature->GetFeatureName(), PakRebuildStatus);
	}

	virtual void ShutdownModule() override
	{
		if (Feature.IsValid())
		{
			IModularFeatures::Get().UnregisterModularFeature(BINARIES_DIFF_PATCH_FEATURE_NAME, Feature.Get());
			Feature.Reset();
		}
	}

private:
	TSharedPtr<FHDiffPatchFeature> Feature;
};

IMPLEMENT_MODULE(FHDiffPatchUEModule, HDiffPatchUE)
