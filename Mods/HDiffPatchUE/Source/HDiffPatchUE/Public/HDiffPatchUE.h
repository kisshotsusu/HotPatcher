// 版权：依据 imzlp 文章 25136 与 12188 重建实现。
// HDiffPatchUE 模块的对外公共 API。
#pragma once

#include "CoreMinimal.h"
#include "BinaryDelta.h"   // FBinaryDelta：字节级 CreateDiff / PatchDiff（文章 25136）
#include "BinaryMerge.h"   // FBinaryMerge：运行时 Pak 构建 / 合并（文章 12188）

// IBinariesDiffPatchFeature 的实现（FHDiffPatchFeature）会在 StartupModule 中自动注册到
// IModularFeatures（特性名 BINARIES_DIFF_PATCH_FEATURE_NAME），一般无需自行实例化它，
// 通过 IModularFeatures 取用即可。
