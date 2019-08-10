// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbleEditorPrivate.h"

#include "AbilityEditor/ablAbilityEditorAddTaskHandlers.h"
#include "AbilityEditor/AblAbilityEditorSettings.h"
#include "AbilityEditor/AblAbilityTaskDetails.h"
#include "AbilityEditor/AblAbilityThumbnailRenderer.h"
#include "AbilityEditor/AssetTypeActions_ablAbilityBlueprint.h"
#include "ablAbility.h"
#include "ablAbilityBlueprint.h"
#include "AbleStyle.h"
#include "ablSettings.h"
#include "AssetToolsModule.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Editor/UnrealEd/Classes/ThumbnailRendering/ThumbnailManager.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#include "IAbleEditor.h"

DEFINE_LOG_CATEGORY(LogAbleEditor);

#define LOCTEXT_NAMESPACE "AbleEditor"

class FAbleEditor : public IAbleEditor
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual uint32 GetAbleAssetCategory() const override { return m_AbleAssetCategory; }

private:
	void RegisterAssetTypes(IAssetTools& AssetTools);
	void RegisterSettings();
	void UnregisterSettings();

	EAssetTypeCategories::Type m_AbleAssetCategory;

	TArray<TSharedPtr<IAssetTypeActions>> m_CreatedAssetTypeActions;

	TSharedPtr<FAblPlayAnimationAddedHandler> m_PlayAnimationTaskHandler;
};

void FAbleEditor::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	m_AbleAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Able")), LOCTEXT("AbleAssetCategory", "Able"));

	FAbleStyle::Initialize();

	RegisterAssetTypes(AssetTools);
	RegisterSettings();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("AblAbilityTask", FOnGetDetailCustomizationInstance::CreateStatic(&FAblAbilityTaskDetails::MakeInstance));

	UThumbnailManager::Get().RegisterCustomRenderer(UAblAbility::StaticClass(), UAblAbilityThumbnailRenderer::StaticClass());
	UThumbnailManager::Get().RegisterCustomRenderer(UAblAbilityBlueprint::StaticClass(), UAblAbilityThumbnailRenderer::StaticClass());

	m_PlayAnimationTaskHandler = MakeShareable(new FAblPlayAnimationAddedHandler());
	m_PlayAnimationTaskHandler->Register();
}


void FAbleEditor::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UAblAbilityBlueprint::StaticClass());
		UThumbnailManager::Get().UnregisterCustomRenderer(UAblAbility::StaticClass());
	}


	FAbleStyle::Shutdown();

	UnregisterSettings();

	// Unregister any customized layout objects.
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("AblAbilityTask");
	}

	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (TSharedPtr<IAssetTypeActions>& Action : m_CreatedAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}
	m_CreatedAssetTypeActions.Empty();
}

void FAbleEditor::RegisterAssetTypes(IAssetTools& AssetTools)
{
	// Register any asset types
	
	// Ability Blueprint
	TSharedRef<IAssetTypeActions> AbilityBlueprint = MakeShareable(new FAssetTypeActions_AblAbilityBlueprint(m_AbleAssetCategory));
	AssetTools.RegisterAssetTypeActions(AbilityBlueprint);
	m_CreatedAssetTypeActions.Add(AbilityBlueprint);


}

void FAbleEditor::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Able",
			LOCTEXT("RuntimeSettingsName", "Able"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Able plugin"),
			GetMutableDefault<UAbleSettings>());

		SettingsModule->RegisterSettings("Editor", "ContentEditors", "AbilityEditor",
			LOCTEXT("AbilityEditorSettingsName", "Ability Editor"),
			LOCTEXT("AbilityEditorSettingsDescription", "Configure the Ability Editor"),
			GetMutableDefault<UAblAbilityEditorSettings>());
	}
}

void FAbleEditor::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "ContentEditors", "AbilityEditor");
		SettingsModule->UnregisterSettings("Project", "Plugins", "Able");
	}
}


IMPLEMENT_MODULE(FAbleEditor, AbleEditor)