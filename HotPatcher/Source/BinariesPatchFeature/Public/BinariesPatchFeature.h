// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Features/IModularFeature.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Misc/EnumRange.h"
#include "BinariesPatchFeature.generated.h"

#define BINARIES_DIFF_PATCH_FEATURE_NAME TEXT("BinariesDiffPatchFeatures")

UENUM(BlueprintType)
enum class EBinariesPatchFeature:uint8
{
	None,
	Count UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EBinariesPatchFeature, EBinariesPatchFeature::Count);

struct BINARIESPATCHFEATURE_API IBinariesDiffPatchFeature: public IModularFeature
{
	virtual ~IBinariesDiffPatchFeature(){};
	virtual bool CreateDiff(const TArray<uint8>& NewData, const TArray<uint8>& OldData, TArray<uint8>& OutPatch) = 0;
	virtual bool PatchDiff(const TArray<uint8>& OldData, const TArray<uint8>& PatchData, TArray<uint8>& OutNewData) = 0;
	virtual FString GetFeatureName()const = 0;

	// File-based apply. The default implementation buffers files into memory (same ~2 GiB ceiling
	// as PatchDiff); streaming-capable features (e.g. HDiffPatchUE) override this to apply directly
	// from disk in fixed-size chunks, removing the memory ceiling for large files.
	// Declared non-pure so existing features keep compiling without change.
	virtual bool PatchDiffToFile(const FString& OldFilePath, const FString& PatchFilePath, const FString& OutNewFilePath);
};

class FBinariesPatchFeatureModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
