// 版权：依据 imzlp 文章 12188（https://imzlp.com/posts/12188/）重建实现。
// 运行时 Pak 创建 / 合并工具。
//
// 重要：本功能依赖引擎内部的 Pak 序列化接口（FPakFile::EncodePakEntriesIntoIndex、FPakInfo），
// 这些接口签名会随 UE 版本漂移。PakRebuilder.cpp 中的实现仅在 HDIFFPATCHUE_ENABLE_PAK_REBUILD=1
// 时编译（见 HDiffPatchUE.Build.cs）。启用前请针对你所用的 UE 5.8 引擎头文件核对
// EncodePakEntriesIntoIndex / FPakInfo 签名并做相应适配。未启用时 BuildPak/MergePaks 仅打日志
// 返回 false，核心的二进制差分功能（文章 25136）不受影响。
#pragma once

#include "CoreMinimal.h"

// 待序列化进 Pak 的单个文件。MountPath 为 UFS 虚拟挂载路径，
// 例如 "../../../PROJECT/Content/Paks/foo.uasset"。
struct FBinaryMergePakEntry
{
	FString MountPath;
	TArray<uint8> Data;
};

struct FBinaryMerge
{
	// 由内存中的条目在 OutputPak 处构建一个 Pak。MountPoint 为各 MountPath 的公共 UFS 前缀，
	// 建索引时会从路径中剥离（默认 "../../../"）。
	static bool BuildPak(const FString& OutputPak, const TArray<FBinaryMergePakEntry>& Entries,
	                     const FString& MountPoint = TEXT("../../../"));

	// 将多个已有的 Pak 文件合并为单个 OutputPak。路径冲突时【先出现的输入 pak 优先】
	//（Pak 挂载顺序语义）。需要读取每个源 pak 的索引，因此与 BuildPak 依赖相同的引擎 Pak 接口。
	static bool MergePaks(const FString& OutputPak, const TArray<FString>& InputPaks,
	                      const FString& MountPoint = TEXT("../../../"));
};
