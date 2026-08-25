// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BinariesPatchFeature.h"
#include "HotPatcherTemplateHelper.hpp"
#include "Misc/FileHelper.h"

#include "Features/IModularFeatures.h"
#include "Misc/EnumRange.h"
#include "Modules/ModuleManager.h"
#include "UObject/Class.h"

DECAL_GETCPPTYPENAME_SPECIAL(EBinariesPatchFeature)

bool IBinariesDiffPatchFeature::PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath)
{
	// 默认实现：把文件缓冲进内存后走 PatchDiff（与旧行为一致，仍受 ~2 GiB 限制）。
	// 流式能力由具体特性（如 HDiffPatchUE）重写提供。
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

void OnBinariesModularFeatureRegistered(const FName& Type, IModularFeature* ModularFeature)
{
	if(!Type.ToString().Equals(BINARIES_DIFF_PATCH_FEATURE_NAME,ESearchCase::IgnoreCase))
		return;
	IBinariesDiffPatchFeature* Feature = static_cast<IBinariesDiffPatchFeature*>(ModularFeature);
	THotPatcherTemplateHelper::AppendEnumeraters<EBinariesPatchFeature>(TArray<FString>{Feature->GetFeatureName()});
}
void OnBinariesModularFeatureUnRegistered(const FName& Type, IModularFeature* ModularFeature)
{
	
}

void FBinariesPatchFeatureModule::StartupModule()
{
	TArray<IBinariesDiffPatchFeature*> RegistedFeatures = IModularFeatures::Get().GetModularFeatureImplementations<IBinariesDiffPatchFeature>(BINARIES_DIFF_PATCH_FEATURE_NAME);
	for(const auto& Featue:RegistedFeatures)
	{
		THotPatcherTemplateHelper::AppendEnumeraters<EBinariesPatchFeature>(TArray<FString>{Featue->GetFeatureName()});
	}
	IModularFeatures::Get().OnModularFeatureRegistered().AddStatic(&OnBinariesModularFeatureRegistered);
	IModularFeatures::Get().OnModularFeatureUnregistered().AddStatic(&OnBinariesModularFeatureUnRegistered);
}

void FBinariesPatchFeatureModule::ShutdownModule()
{
	
}

IMPLEMENT_MODULE( FBinariesPatchFeatureModule, BinariesPatchFeature );
