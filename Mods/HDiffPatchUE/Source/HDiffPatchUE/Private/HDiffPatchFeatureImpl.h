// Copyright (c) rebuilt per imzlp article 25136. See BinaryDelta.h / HDiffPatchUE module.
#pragma once

#include "BinariesPatchFeature.h" // IBinariesDiffPatchFeature, BINARIES_DIFF_PATCH_FEATURE_NAME
#include "BinaryDelta.h"

// Concrete IBinariesDiffPatchFeature backed by the self-contained FBinaryDelta.
class FHDiffPatchFeature : public IBinariesDiffPatchFeature
{
public:
	virtual bool CreateDiff(const TArray<uint8>& NewData, const TArray<uint8>& OldData, TArray<uint8>& OutPatch) override;
	virtual bool PatchDiff(const TArray<uint8>& OldData, const TArray<uint8>& PatchData, TArray<uint8>& OutNewData) override;
	virtual bool PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath) override;
	virtual FString GetFeatureName() const override;
};
