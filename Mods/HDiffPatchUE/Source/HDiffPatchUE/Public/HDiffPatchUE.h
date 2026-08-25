// Copyright (c) rebuilt per imzlp articles 25136 & 12188.
// Public API of the HDiffPatchUE module.
#pragma once

#include "CoreMinimal.h"
#include "BinaryDelta.h"   // FBinaryDelta : byte-level CreateDiff / PatchDiff
#include "BinaryMerge.h"   // FBinaryMerge : runtime Pak build / merge (article 12188)

// The IBinariesDiffPatchFeature implementation (registered automatically in StartupModule)
// is reached through IModularFeatures under BINARIES_DIFF_PATCH_FEATURE_NAME; you normally
// do not instantiate FHDiffPatchFeature yourself.
