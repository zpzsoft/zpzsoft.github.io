// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PathFollowerEditor.h"
#include "AssetToolsModule.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Editor/UnrealEd/Classes/ThumbnailRendering/ThumbnailManager.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "FAssetTypeActions_PathFollowerBlueprint.h"

#define LOCTEXT_NAMESPACE "FPathFollowerEditorModule"

void FPathFollowerEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type m_AbleAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Path Follower")), LOCTEXT("PathFollowerAssetCategory", "Path Follower"));
	TSharedRef<IAssetTypeActions> AbilityBlueprint = MakeShareable(new FAssetTypeActions_PathFollowerBlueprint(m_AbleAssetCategory));
	AssetTools.RegisterAssetTypeActions(AbilityBlueprint);
}

void FPathFollowerEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPathFollowerEditorModule, PathFollowerEditor)