// Copyright 2019 Lipeng Zha, Inc. All Rights Reserved.

#pragma once

class FObjectPreSaveContext;

	#include "ToolMenuContext.h"
	#include "ToolMenu.h"

#include "HotPatcherSettings.h"
#include "ETargetPlatform.h"
#include "FlibHotPatcherCoreHelper.h"
#include "HotPatcherSettings.h"
#include "CreatePatch/FExportPatchSettings.h"
#include "CreatePatch/FExportReleaseSettings.h"
#include "Model/FHotPatcherContextBase.h"
#include "HotPatcherActionManager.h"
#include "HotPatcherCore.h"
// engine header
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserDelegates.h"
#include "MissionNotificationProxy.h"
#include "ThreadUtils/FProcWorkerThread.hpp"

	#define InvokeTab TryInvokeTab

DECLARE_LOG_CATEGORY_EXTERN(LogHotPatcherEdotor,All,All)

class FToolBarBuilder;
class FMenuBuilder;
class FExtender;
class FAssetTypeActions_PrimaryAssetLabel;


struct FContentBrowserSelectedInfo
{
	TArray<FAssetData> SelectedAssets;
	TArray<FString> SelectedPaths;
	TArray<FName> OutAllAssetPackages;
};

struct HOTPATCHEREDITOR_API FSHotPatcherContext
{
	FString Category;
	FString ActionName;
};

class HOTPATCHEREDITOR_API FHotPatcherEditorModule : public IModuleInterface
{
public:
	static FHotPatcherEditorModule& Get();
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	void OpenDockTab();
	/** This function will be bound to Command. */
	void PluginButtonClicked(const FSHotPatcherContext& Context);
private: // for slate
	void AddToolbarExtension(FToolBarBuilder& Builder);
	TSharedRef<SWidget> HandlePickingModeContextMenu();
	TSharedRef<class SDockTab> SpawnHotPatcherTab(const FSHotPatcherContext& Context);
	void AddMenuExtension(FMenuBuilder& Builder);

	void OnTabClosed(TSharedRef<SDockTab> InTab);
	TArray<FAssetData> GetSelectedAssetsInBrowserContent();
	TArray<FString> GetSelectedFolderInBrowserContent();
public:
	TSharedPtr<FProcWorkerThread> CreateProcMissionThread(const FString& Bin, const FString& Command, const FString& MissionName);
	TSharedPtr<FProcWorkerThread> RunProcMission(const FString& Bin, const FString& Command, const FString& MissionName, const FText& NotifyTextOverride = FText{});

	
	void CreateRootMenu();
	void CreateAssetContextMenu(FToolMenuSection& InSection);
	void ExtendContentBrowserAssetSelectionMenu();
	void ExtendContentBrowserPathSelectionMenu();
	void MakeCookActionsSubMenu(UToolMenu* Menu);
	void MakeCookAndPakActionsSubMenu(UToolMenu* Menu);
	void MakeHotPatcherPresetsActionsSubMenu(UToolMenu* Menu);
	void OnAddToPatchSettings(const FToolMenuContext& MenuContent);
	TArray<ETargetPlatform> GetAllowCookPlatforms() const;
	static void OnCookPlatformForExterner(ETargetPlatform Platform);;
	void OnCookPlatform(ETargetPlatform Platform);
	void OnCookAndPakPlatform(ETargetPlatform Platform, bool bAnalysicDependencies);
	void CookAndPakByAssetsAndFilters(TArray<FPatcherSpecifyAsset> IncludeAssets,TArray<FDirectoryPath> IncludePaths,TArray<ETargetPlatform> Platforms,bool bForceStandalone = false);
	void CookAndPakByPatchSettings(TSharedPtr<FExportPatchSettings> PatchSettings,bool bForceStandalone);
	void OnPakPreset(FExportPatchSettings Config,ETargetPlatform Platform);
	void OnPakPreset(FExportPatchSettings Config);
	void OnObjectPreSave(UObject* ObjectSaved, FObjectPreSaveContext SaveContext);


	FExportPatchSettings MakeTempPatchSettings(
		const FString& Name,
		const TArray<FDirectoryPath>& AssetIncludeFilters,
		const TArray<FPatcherSpecifyAsset>& IncludeSpecifyAssets,
		const TArray<FPlatformExternAssets>& ExternFiles,
		const TArray<ETargetPlatform>& PakTargetPlatforms,
		bool bCook
	);
private:
	TArray<ETargetPlatform> GetAllCookPlatforms()const;
	
private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<FExtender> MenuExtender;
	TSharedPtr<FExtender> ToolbarExtender;
	TSharedPtr<SDockTab> DockTab;
	mutable TSharedPtr<FProcWorkerThread> mProcWorkingThread;
	UMissionNotificationProxy* MissionNotifyProay = NULL;
	TSharedPtr<FExportPatchSettings> PatchSettings;
	TArray<class UPatcherProxy*> Proxys;
	FString StyleSetName;
	TSharedPtr<FAssetTypeActions_PrimaryAssetLabel> AssetTypeActions;
};

