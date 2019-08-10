#include "FPathFollowerEditor.h"

#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "ClassIconFinder.h"
#include "Components/ActorComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Delegates/MulticastDelegateBase.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Logging/LogMacros.h"
#include "GameFramework/WorldSettings.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ScopedTransaction.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "UObject/Object.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "Kismet/Public/SSCSEditor.h"

#if WITH_EDITOR
#include "Editor.h"
#include "EditorStyleSet.h"
#include "EditorReimportHandler.h"
#include "Editor/Kismet/Public/SBlueprintEditorToolbar.h"
#endif

void FPathFollowerEditor::InitPathFollowerEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InAbilityBlueprints, bool ShouldUseDataOnlyEditor)
{
	TArray<UObject*> ObjectsBeingEditted;
	for (UBlueprint* BP : InAbilityBlueprints)
	{
		ObjectsBeingEditted.Add(BP);
	}

	// Initialize the asset editor and spawn tabs
	const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

#if WITH_EDITOR
	InitBlueprintEditor(Mode, InitToolkitHost, InAbilityBlueprints, false);
#endif
}

void FPathFollowerEditor::AddComponent(UClass* NewComponentClass, FName ComponentName)
{
#if WITH_EDITOR
	GetSCSEditor()->AddNewComponent(NewComponentClass, nullptr, false, false);
#endif
}
