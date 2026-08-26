// Copyright (c) rebuilt per imzlp article 25136.
#include "Modules/ModuleManager.h"
#include "Features/IModularFeatures.h"
#include "Logging/LogMacros.h" // DEFINE_LOG_CATEGORY_STATIC
#include "Misc/FileHelper.h"
#include "HDiffPatchFeatureImpl.h"

DEFINE_LOG_CATEGORY_STATIC(LogHDiffPatchUE, Log, All);

// IBinariesDiffPatchFeature::PatchDiffToFile 的默认实现定义于 BinariesPatchFeature 模块，
// 但该接口未导出符号，跨模块链接会失败（LNK2001）。在此提供一份本地定义满足 vtable 引用。
// 语义与上游一致：整文件读入内存后走 PatchDiff（受 ~2 GiB 上限），仅作兜底；本插件实际
// 使用 FHDiffPatchFeature::PatchDiffToFile 的流式重写。
bool IBinariesDiffPatchFeature::PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath)
{
	TArray<uint8> OldData, PatchData, NewData;
	if (!FFileHelper::LoadFileToArray(OldData, *OldFilePath))
	{
		return false;
	}
	if (!FFileHelper::LoadFileToArray(PatchData, *PatchFilePath))
	{
		return false;
	}
	if (!PatchDiff(OldData, PatchData, NewData) || NewData.Num() == 0)
	{
		return false;
	}
	return FFileHelper::SaveArrayToFile(NewData, *OutNewFilePath);
}

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
