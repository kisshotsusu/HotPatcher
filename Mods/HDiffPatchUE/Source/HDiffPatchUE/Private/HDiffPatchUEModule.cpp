// Copyright (c) rebuilt per imzlp article 25136.
#include "Modules/ModuleManager.h"
#include "Features/IModularFeatures.h"
#include "HDiffPatchFeatureImpl.h"

DEFINE_LOG_CATEGORY_STATIC(LogHDiffPatchUE, Log, All);

class FHDiffPatchUEModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		Feature = MakeShared<FHDiffPatchFeature>();
		// Register under the same Modular Feature name HotPatcher's GenerateBinariesPatch queries.
		IModularFeatures::Get().RegisterModularFeature(BINARIES_DIFF_PATCH_FEATURE_NAME, Feature.Get());
		UE_LOG(LogHDiffPatchUE, Log,
			TEXT("HDiffPatchUE (self-contained binary delta) registered as '%s'. "
			     "Binary merge for .pak and .utoc/.ucas is now available to HotPatcher BinariesPatch."),
			*Feature->GetFeatureName());
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
