// Copyright (c) rebuilt per imzlp article 25136.
#include "HDiffPatchFeatureImpl.h"

bool FHDiffPatchFeature::CreateDiff(const TArray<uint8>& NewData, const TArray<uint8>& OldData, TArray<uint8>& OutPatch)
{
	return FBinaryDelta::CreateDiff(NewData, OldData, OutPatch);
}

bool FHDiffPatchFeature::PatchDiff(const TArray<uint8>& OldData, const TArray<uint8>& PatchData, TArray<uint8>& OutNewData)
{
	return FBinaryDelta::PatchDiff(OldData, PatchData, OutNewData);
}

bool FHDiffPatchFeature::PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath)
{
	return FBinaryDelta::PatchDiffToFile(OldFilePath, PatchFilePath, OutNewFilePath);
}

FString FHDiffPatchFeature::GetFeatureName() const
{
	// 与原 hxhb/HDiffPatchUE 暴露的特性名保持一致，确保 HotPatcher 中已有的
	// “Binaries Patch Type” 选择 / BinariesPatchFeatureName 设置继续可用。
	return TEXT("HDiffPatchUE");
}
