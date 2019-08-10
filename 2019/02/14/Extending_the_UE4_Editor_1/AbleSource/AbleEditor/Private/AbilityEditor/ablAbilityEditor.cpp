// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AbilityEditor/ablAbilityEditor.h"

#include "AbleEditorPrivate.h"

#include "ablAbility.h"
#include "ablAbilityBlueprint.h"
#include "ablAbilityComponent.h"
#include "ablAbilityDebug.h"
#include "AbilityEditor/ablAbilityEditorModes.h"
#include "AbilityEditor/ablAbilityEditorCommands.h"
#include "AbilityEditor/AblAbilityEditorSettings.h"
#include "AbilityEditor/ablAbilityValidator.h"
#include "AbilityEditor/SAbilityEditorToolbar.h"
#include "AbilityEditor/SAbilitySelectPreviewAssetDlg.h"
#include "AbilityEditor/SAbilityTaskPicker.h"
#include "AbilityEditor/SAbilityValidatorResultsDlg.h"
#include "AbilityEditor/SAbilityViewport.h"
#include "AbleEditorEventManager.h"
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
#include "Editor.h"
#include "EditorStyleSet.h"
#include "EditorReimportHandler.h"
#include "Editor/Kismet/Public/SBlueprintEditorToolbar.h"
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

#define LOCTEXT_NAMESPACE "AblAbilitiesEditor"

FAblAbilityEditor::FAblAbilityEditor()
	: FBlueprintEditor(),
	m_AssetDirtyBrush(nullptr),
	m_TabIcon(nullptr),
	m_PreviewScene(FAbilityEditorPreviewScene::ConstructionValues().AllowAudioPlayback(true).ShouldSimulatePhysics(true)),
	m_IsPaused(false),
	m_DisplayTaskCost(false),
	m_DrawCharacterCollision(true),
	m_DrawArrowComponent(true),
	m_DrawCameraComponent(true)
{
	m_EditorSettings = GetMutableDefault<UAblAbilityEditorSettings>(UAblAbilityEditorSettings::StaticClass());
	m_Validator = GetMutableDefault<UAblAbilityValidator>(UAblAbilityValidator::StaticClass());

	check(m_EditorSettings);

	// Slightly annoying/obfuscated setting. This setting can prevent particles from being drawn - not sure why.
	// Persona calls this out but only mentions that it's attached effects but it's all effects.
	m_PreviewScene.GetWorld()->GetWorldSettings()->SetIsTemporarilyHiddenInEditor(false);
}

FAblAbilityEditor::~FAblAbilityEditor()
{
	FEditorDelegates::OnAssetPostImport.RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);
}

void FAblAbilityEditor::InitAbilityEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray<UBlueprint*>& InAbilityBlueprints, bool ShouldUseDataOnlyEditor)
{
	check(InAbilityBlueprints.Num() > 0);

	if (!m_AssetDirtyBrush)
	{
		m_AssetDirtyBrush = FEditorStyle::GetBrush("ContentBrowser.ContentDirty");
	}

	if (!m_TabIcon)
	{
		m_TabIcon = FAbleStyle::GetBrush("Able.Tabs.AbilityTimeline");
	}

	FAblAbilityEditorCommands::Register();

	BindCommands();

	// Init stuff.
	TSharedPtr<FAblAbilityEditor> Editor(SharedThis(this));

	if (!Toolbar.IsValid())
	{
		Toolbar = MakeShareable(new FBlueprintEditorToolbar(SharedThis(this)));
	}

	if (!m_AbilityEditorToolbar.IsValid())
	{
		m_AbilityEditorToolbar = MakeShareable(new FAblAbilityEditorToolbar());
	}

	TArray<UObject*> ObjectsBeingEditted;
	for (UBlueprint* BP : InAbilityBlueprints)
	{
		ObjectsBeingEditted.Add(BP);
	}

	// Modes
	TSharedRef<FApplicationMode> BlueprintMode = MakeShareable(new FAppAbilityEditorBlueprintMode(Editor));
	TSharedRef<FApplicationMode> TimelineMode = MakeShareable(new FAppAbilityEditorTimelineMode(Editor));

	AddApplicationMode(TimelineMode->GetModeName(), TimelineMode);
	AddApplicationMode(BlueprintMode->GetModeName(), BlueprintMode);

	// Initialize the asset editor and spawn tabs
	const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, FAblAbilityEditorModes::AblAbilityEditorName, DummyLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectsBeingEditted);

	CommonInitialization(InAbilityBlueprints);

	// Custom Menu/Toolbar
	ExtendMenu();
	ExtendToolbar();

	// Regenerate everything.
	RegenerateMenusAndToolbars();

	// This does the actual layout generation.
	SetCurrentMode(TimelineMode->GetModeName());

	PostLayoutBlueprintEditorInitialization();

	FAbleEditorEventManager::Get().BroadcastAbilityEditorInstantiated(*this);
}

void FAblAbilityEditor::Reinitialize()
{
	if (GetCurrentMode() == FAblAbilityEditorModes::AbilityTimelineMode)
	{
		if (GetEditorSettings().m_PreviewAsset.IsValid())
		{
			if (!m_PreviewActor.IsValid())
			{
				SpawnPreviewActor();
			}
		}
		else
		{
			// We don't have an asset selected in our settings, go ahead and ask the user for one.
			SetPreviewAsset();
		}
	}
}

FName FAblAbilityEditor::GetToolkitFName() const
{
	return FName("AblAbilitiesEditor");
}

FText FAblAbilityEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AblAbilitiesEditorAppLabel", "Ability Editor");
}

FText FAblAbilityEditor::GetToolkitName() const
{
	const TArray<UObject *>& CurrentEditingObjects = GetEditingObjects();

	check(CurrentEditingObjects.Num() > 0);

	FFormatNamedArguments Args;

	const UObject* EditingObject = CurrentEditingObjects[0];

	const bool bDirtyState = EditingObject->GetOutermost()->IsDirty();

	Args.Add(TEXT("ObjectName"), FText::FromString(EditingObject->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("AblAbilitiesToolkitName", "{ObjectName}{DirtyState}"), Args);
}

FText FAblAbilityEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetEditingObject();

	check(EditingObject != NULL);

	return FAssetEditorToolkit::GetToolTipTextForObject(EditingObject);
}

FString FAblAbilityEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("AblAbilitiesEditor");
}

FLinearColor FAblAbilityEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

UBlueprint* FAblAbilityEditor::GetBlueprintObj() const
{
	for (UObject* EditingObject : GetEditingObjects())
	{
		if (EditingObject->IsA<UAblAbilityBlueprint>())
		{
			return CastChecked<UBlueprint>(EditingObject);
		}
	}

	return nullptr;
}

UAblAbilityBlueprint* FAblAbilityEditor::GetAbilityBlueprint() const
{
	if (UBlueprint* AbilityBlueprint = GetBlueprintObj())
	{
		return Cast<UAblAbilityBlueprint>(AbilityBlueprint);
	}

	return nullptr;
}

UObject* FAblAbilityEditor::GetAbilityBlueprintAsObject() const
{
	if (UAblAbilityBlueprint* Blueprint = GetAbilityBlueprint())
	{
		return Cast<UObject>(Blueprint);
	}

	return nullptr;
}

UAblAbility* FAblAbilityEditor::GetAbility() const
{
	if (UAblAbilityBlueprint* AbilityBlueprint = GetAbilityBlueprint())
	{
		check(AbilityBlueprint->GeneratedClass);
		return Cast<UAblAbility>(AbilityBlueprint->GeneratedClass->GetDefaultObject());
	}

	return nullptr;
}

UAblAbilityTask* FAblAbilityEditor::GetCurrentlySelectedAbilityTask() const
{
	return m_CurrentTask.IsValid() ? m_CurrentTask.Get() : nullptr;
}

void FAblAbilityEditor::ToggleDependencyOnTask(UAblAbilityTask* Task, const UAblAbilityTask* DependsOn)
{
	if (Task && DependsOn)
	{
		const FScopedTransaction ToggleDependency(LOCTEXT("ToggleDependencyTransaction", "Toggle Dependency"));

		Task->Modify();

		const TArray<const UAblAbilityTask*>& Dependencies = Task->GetTaskDependencies();
		if (Dependencies.Contains(DependsOn))
		{
			Task->RemoveDependency(DependsOn);
		}
		else
		{
			Task->AddDependency(DependsOn);
		}
	}
}

bool FAblAbilityEditor::TaskHasDependency(const UAblAbilityTask* Task, const UAblAbilityTask* DependsOn) const
{
	if (Task && DependsOn)
	{
		return Task->GetTaskDependencies().Contains(DependsOn);
	}

	return false;
}

const UAblAbility* FAblAbilityEditor::GetConstAbility() const
{
	return GetAbility();
}

void FAblAbilityEditor::Tick(float DeltaTime)
{
	FBlueprintEditor::Tick(DeltaTime);

	if (m_Viewport.IsValid())
	{
		m_Viewport.Pin()->RefreshViewport();
	}

	if (m_PreviewActor.IsValid())
	{
		if (USkeletalMeshComponent* SkeletalComponent = m_PreviewActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (SkeletalComponent->GetAnimationMode() == EAnimationMode::AnimationBlueprint)
			{
				// Manually Update our AnimBlueprint
				if (UAnimInstance* AnimationInstance = SkeletalComponent->GetAnimInstance())
				{
					AnimationInstance->BlueprintUpdateAnimation(DeltaTime);
				}
			}
		}

		if (UMeshComponent* MeshComponent = m_PreviewActor->FindComponentByClass<UMeshComponent>())
		{
			MeshComponent->SetRelativeRotation(GetEditorSettings().m_MeshRotation);
		}
	}

	if (!IsPlayingAbility() && IsPaused())
	{
		// Reset our Pause state.
		m_IsPaused = false;
	}
}

TStatId FAblAbilityEditor::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FAblAbilityEditor, STATGROUP_Tickables);
}

const FSlateBrush* FAblAbilityEditor::GetDirtyImageForMode(FName Mode) const
{
	// Right now, both modes edit the same object. So, just return the brush if any assets are dirty.
	for (UObject* Obj : GetEditingObjects())
	{
		if (UPackage* package = Obj->GetOutermost())
		{
			if (package->IsDirty())
			{
				return m_AssetDirtyBrush;
			}
		}
	}

	return nullptr;
}

void FAblAbilityEditor::AddNewTask()
{
	if (UAblAbility* Ability = GetAbility())
	{
		TSharedRef<SAblAbilityTaskPicker> Dialog = SNew(SAblAbilityTaskPicker);
		if (Dialog->DoModal())
		{
			if (const UAblAbilityTask* DefaultObject = Dialog->GetTaskClass().GetDefaultObject())
			{
				if (UClass* taskClass = DefaultObject->GetClass())
				{
					const FScopedTransaction AddTaskTransaction(LOCTEXT("AddTaskTransaction", "Add Task"));

					Ability->Modify();

					UAblAbilityTask* NewTask = NewObject<UAblAbilityTask>(Ability, taskClass, NAME_None, RF_Transactional);
					
					Ability->AddTask(*NewTask);

					FAbleEditorEventManager& EventMgr = FAbleEditorEventManager::Get();
					
					// Fire off our Events to any listeners.
					EventMgr.BroadcastAnyAbilityTaskCreated(*this, *NewTask);
					EventMgr.BroadcastAbilityTaskCreated(*this, *NewTask);

					RefreshEditors();
				}
			}
		}
	}
}

void FAblAbilityEditor::RemoveCurrentTask()
{
	if (UAblAbilityTask* CurrentTask = GetCurrentlySelectedAbilityTask())
	{
		RemoveTask(*CurrentTask);
	}
}

void FAblAbilityEditor::RemoveTask(UAblAbilityTask& Task)
{
	if (UAblAbility* Ability = GetAbility())
	{
		const FScopedTransaction RemoveTaskTransaction(LOCTEXT("RemoveTaskTransaction", "Remove Task"));

		Ability->Modify();

		Ability->RemoveTask(Task);

		RefreshEditors();
	}

	m_CurrentTask.Reset();
}

void FAblAbilityEditor::SetCurrentTask(UAblAbilityTask* Task)
{
	m_CurrentTask = Task;
}

void FAblAbilityEditor::ResizeAbility()
{
	if (UAblAbility* Ability = GetAbility())
	{
		const FScopedTransaction ResizeTransaction(LOCTEXT("ResizeAbilityTransaction", "Resize Ability"));

		float MaxTime = 0.0f;
		const TArray<UAblAbilityTask*>& Tasks = Ability->GetTasks();
		for (const UAblAbilityTask* Task : Tasks)
		{
			MaxTime = FMath::Max(MaxTime, Task->GetEndTime());
		}

		Ability->Modify();

		Ability->SetLength(MaxTime);
		
		RefreshEditors();
	}
}

void FAblAbilityEditor::Validate() const
{
	if (m_Validator.IsValid())
	{
		m_Validator->Validate(*GetAbility());

		// Create and Display Modal window.
		TSharedRef<SAblAbilityValidatorResultsDlg> ResultsDlg = SNew(SAblAbilityValidatorResultsDlg).Validator(m_Validator);
		ResultsDlg->DoModal();
	}
}

FVector2D FAblAbilityEditor::GetAbilityTimeRange() const
{
	float EndTime = 1.0f;
	if (const UAblAbility* Ability = GetConstAbility())
	{
		EndTime = Ability->GetLength();
	}

	return FVector2D(0.0f, EndTime);
}

FLinearColor FAblAbilityEditor::GetTaskCostColor(const UAblAbilityTask& Task) const
{
	const float ClampedValue = FMath::Clamp(Task.GetEstimatedTaskCost(), 0.0f, 1.0f);
	return FLinearColor::LerpUsingHSV(FLinearColor::Green, FLinearColor::Red, ClampedValue / 1.0f );
}

bool FAblAbilityEditor::IsDrawingCollisionQueries() const
{
	return FAblAbilityDebug::ShouldDrawQueries();
}

bool FAblAbilityEditor::IsPlayingAbility() const
{
	if (m_PreviewAbilityComponent.IsValid())
	{
		return m_PreviewAbilityComponent->IsPlayingAbility();
	}

	return false;
}

void FAblAbilityEditor::SetAbilityPreviewTime(float Time)
{
	if (m_PreviewAbilityComponent.IsValid())
	{
		if (!m_PreviewAbilityComponent->IsPlayingAbility())
		{
			m_PreviewAbilityComponent->PlayAbilityFromEditor(GetAbility()); // Need an Ability to skip through it.
			PauseAbility(); // Make sure we're paused as well.
		}

		m_PreviewAbilityComponent->SetAbilityTime(Time);

		if (m_Viewport.IsValid())
		{
			TSharedPtr<SAbilityEditorViewport> ViewportWidget = m_Viewport.Pin()->GetViewportWidget();
			if (ViewportWidget.IsValid())
			{
				TSharedPtr<FAbilityEditorViewportClient> ViewportClient = ViewportWidget->GetAbilityViewportClient();
				if (ViewportClient.IsValid())
				{
					ViewportClient->TickWorld(0.00001); // Tick just a small amount to make sure everything gets updated.
				}
			}
		}
	}
}

float FAblAbilityEditor::GetAbilityLength() const
{
	if (const UAblAbility* CurrentAbility = GetAbility())
	{
		return CurrentAbility->GetLength();
	}

	return 0.0f;
}

float FAblAbilityEditor::GetAbilityCurrentTime() const
{
	if (m_PreviewAbilityComponent.IsValid())
	{
		return m_PreviewAbilityComponent->GetCurrentAbilityTime();
	}

	return 0.0f;
}

void FAblAbilityEditor::CreatePreviewActor(UWorld* World)
{
	UObject* SettingsPreviewAsset = GetDefault<UAblAbilityEditorSettings>(UAblAbilityEditorSettings::StaticClass())->m_PreviewAsset.TryLoad();
	if (!SettingsPreviewAsset)
	{
		return;
	}
	
	if (SettingsPreviewAsset->IsA<UBlueprint>() && !SettingsPreviewAsset->IsA<UAnimBlueprint>())
	{
		SpawnPreviewActorFromBlueprint(World);
	}
	else
	{
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Name = MakeUniqueObjectName(World, APawn::StaticClass());
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ActorSpawnParameters.ObjectFlags = EObjectFlags::RF_Transient; // We don't want to save anything on this object.	

		AActor* NewActor = Cast<AActor>(World->SpawnActor<APawn>(ActorSpawnParameters));
		NewActor->SetActorTickEnabled(true);

		USceneComponent* SceneComponent = NewActor->FindComponentByClass<USceneComponent>();
		if (!SceneComponent)
		{
			// Give the new actor a root scene component, so we can attach multiple sibling components to it
			SceneComponent = NewObject<USceneComponent>(NewActor);
			NewActor->AddOwnedComponent(SceneComponent);
			NewActor->SetRootComponent(SceneComponent);
			SceneComponent->RegisterComponent();
		}

		UArrowComponent* ArrowComponent = NewActor->FindComponentByClass<UArrowComponent>();
		if (!ArrowComponent && m_DrawArrowComponent)
		{
			// Arrow Component so we can see our facing.
			ArrowComponent = NewObject<UArrowComponent>(NewActor);
			NewActor->AddOwnedComponent(ArrowComponent);
			ArrowComponent->RegisterComponent();
		}
		else if (ArrowComponent && !m_DrawArrowComponent)
		{
			if (FSceneInterface* Scene = ArrowComponent->GetScene())
			{
				Scene->RemovePrimitive(ArrowComponent);
			}
		}

		UAblAbilityComponent* AbilityComponent = NewActor->FindComponentByClass<UAblAbilityComponent>();
		if (!AbilityComponent)
		{
			// Add our Ability Component
			AbilityComponent = NewObject<UAblAbilityComponent>(NewActor);
			NewActor->AddOwnedComponent(AbilityComponent);
			AbilityComponent->RegisterComponent();
		}

		if (SettingsPreviewAsset->IsA<UStaticMesh>())
		{
			UStaticMeshComponent* StaticComponent = NewActor->FindComponentByClass<UStaticMeshComponent>();
			if (!StaticComponent)
			{
				StaticComponent = NewObject<UStaticMeshComponent>(NewActor);
				NewActor->AddOwnedComponent(StaticComponent);
				StaticComponent->SetupAttachment(NewActor->FindComponentByClass<USceneComponent>());
				StaticComponent->SetStaticMesh(CastChecked<UStaticMesh>(SettingsPreviewAsset));
				StaticComponent->RegisterComponent();
			}
		}
		else if (SettingsPreviewAsset->IsA<USkeletalMesh>() || SettingsPreviewAsset->IsA<UAnimBlueprint>())
		{
			USkeletalMeshComponent* SkeletalComponent = NewActor->FindComponentByClass<USkeletalMeshComponent>();
			if (!SkeletalComponent)
			{
				SkeletalComponent = NewObject<USkeletalMeshComponent>(NewActor);
				NewActor->AddOwnedComponent(SkeletalComponent);
				SkeletalComponent->SetupAttachment(NewActor->FindComponentByClass<USceneComponent>());
				SkeletalComponent->RegisterComponent();
			}

			USkeletalMesh* SkeletalMesh = nullptr;
			UAnimBlueprint* AnimationBP = Cast<UAnimBlueprint>(SettingsPreviewAsset);

			if (AnimationBP)
			{
				SkeletalMesh = AnimationBP->TargetSkeleton->GetPreviewMesh(true);
			}
			else
			{
				SkeletalMesh = CastChecked<USkeletalMesh>(SettingsPreviewAsset);
			}

			SkeletalComponent->SetSkeletalMesh(SkeletalMesh);
			if (AnimationBP)
			{
				SkeletalComponent->SetAnimInstanceClass(AnimationBP->GeneratedClass);
			}
		}

		m_PreviewActor = NewActor;
		m_PreviewAbilityComponent = AbilityComponent;
	}
}

void FAblAbilityEditor::SetPreviewAsset()
{
	TSharedPtr<SAblAbilitySelectPreviewAssetDlg> PickerDlg = SNew(SAblAbilitySelectPreviewAssetDlg);
	if (PickerDlg->DoModal())
	{
		const TWeakObjectPtr<UObject>& SelectedAsset = PickerDlg->GetPreviewAsset();
		if (SelectedAsset.IsValid())
		{
			GetEditorSettings().m_PreviewAsset = FSoftObjectPath(SelectedAsset.Get());
			GetEditorSettings().SaveConfig();

			SpawnPreviewActor();
		}
	}
}

const FSlateBrush* FAblAbilityEditor::GetDefaultTabIcon() const
{
	if (m_TabIcon)
	{
		return m_TabIcon;
	}
	return FWorkflowCentricApplication::GetDefaultTabIcon();
}

TSharedRef<SWidget> FAblAbilityEditor::GenerateTaskContextMenu()
{
	FMenuBuilder MenuBuilder(true, GetToolkitCommands());
	MenuBuilder.BeginSection("Ability", LOCTEXT("AbilitySectionHeader", "Ability Actions"));

	MenuBuilder.AddMenuEntry(FAblAbilityEditorCommands::Get().m_AddTask);
	MenuBuilder.AddMenuEntry(FAblAbilityEditorCommands::Get().m_RemoveTask);
	MenuBuilder.EndSection();

	if (GetCurrentlySelectedAbilityTask())
	{
		MenuBuilder.BeginSection("Ability", LOCTEXT("TaskHeader", "Task"));
		MenuBuilder.AddSubMenu(LOCTEXT("DependencyLabel", "Dependency"),
			LOCTEXT("DependencyTooltip", "Select any Tasks that this Task requires be completed before executing."),
			FNewMenuDelegate::CreateRaw(this, &FAblAbilityEditor::CreateTaskDependencyMenu),
			false,
			FSlateIcon());
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

void FAblAbilityEditor::CreateTaskDependencyMenu(FMenuBuilder& MenuBuilder)
{
	FAblAbilityEditor* AbilityEditor = this;
	UAblAbilityTask* CurrentSelectedTask = GetCurrentlySelectedAbilityTask();
	const UAblAbility* Ability = GetConstAbility();
	const TArray<UAblAbilityTask*>& AllTasks = Ability->GetTasks();

	for (UAblAbilityTask* Task : AllTasks)
	{
		if (Task == CurrentSelectedTask)
		{
			continue; // Can't be dependent on yourself.
		}

		const FText TaskName = AbilityEditor->GetEditorSettings().m_ShowDescriptiveTaskTitles ? Task->GetDescriptiveTaskName() : FText::FromString(Task->GetName());

		MenuBuilder.AddMenuEntry(TaskName,
			FText::FormatOrdered(LOCTEXT("TaskDependencyTooltip", "Toggle a Dependency to {0}"), TaskName),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([=] { AbilityEditor->ToggleDependencyOnTask(CurrentSelectedTask, Task); }),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([=] { return AbilityEditor->TaskHasDependency(CurrentSelectedTask, Task); })
			),
			NAME_None,
			EUserInterfaceActionType::Check);
	}
}

void FAblAbilityEditor::GetSaveableObjects(TArray<UObject*>& OutObjects) const
{
	// We're about to save. Go ahead and sort our tasks to maintain order.
	GetAbility()->SortTasks();

	OutObjects.Append(GetEditingObjects());
}

void FAblAbilityEditor::ExtendMenu()
{
	if (m_MenuExtender.IsValid())
	{
		RemoveMenuExtender(m_MenuExtender);
		m_MenuExtender.Reset();
	}

	m_MenuExtender = MakeShareable(new FExtender);

	FKismet2Menu::SetupBlueprintEditorMenu(m_MenuExtender, *this);

	// Custom Menus for Ability Editor
	{
		struct Local
		{
			static void AddAbilityFileMenu(FMenuBuilder& MenuBuilder)
			{
				// View
				MenuBuilder.BeginSection("FAblAbilityEditor", LOCTEXT("AbilityEditorMenu_File", "Blueprint"));
				{
				}
				MenuBuilder.EndSection();
			}

			static void AddAbilityTimelineMenu(FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.BeginSection("EditTimeline", LOCTEXT("AbilityEditorMenu_Timeline", "Timeline"));
				{
					MenuBuilder.AddMenuEntry(FAblAbilityEditorCommands::Get().m_ToggleShowCollisionQueries);
					MenuBuilder.AddMenuSeparator();
					MenuBuilder.AddMenuEntry(FAblAbilityEditorCommands::Get().m_ToggleDrawArrowComponent);
					MenuBuilder.AddMenuEntry(FAblAbilityEditorCommands::Get().m_ToggleDrawCharacterCollision);
					MenuBuilder.AddMenuEntry(FAblAbilityEditorCommands::Get().m_ToggleDrawCameraComponent);
				}
				MenuBuilder.EndSection();
			}

			static void AddAbilityTimelineDropdownMenu(FMenuBarBuilder& MenuBarBuilder)
			{
				MenuBarBuilder.AddPullDownMenu(LOCTEXT("Timeline", "Timeline"),
					LOCTEXT("Timeline_Tooltip", "Opens the Timeline Menu"),
					FNewMenuDelegate::CreateStatic(&Local::AddAbilityTimelineMenu),
					"Timeline");
			}
		};


		m_MenuExtender->AddMenuBarExtension(
			"Edit",
			EExtensionHook::After,
			GetToolkitCommands(),
			FMenuBarExtensionDelegate::CreateStatic(&Local::AddAbilityTimelineDropdownMenu));
	}

	AddMenuExtender(m_MenuExtender);	
}

void FAblAbilityEditor::ExtendToolbar()
{
	// If the ToolbarExtender is valid, remove it before rebuilding it
	if (m_ToolbarExtender.IsValid())
	{
		RemoveToolbarExtender(m_ToolbarExtender);
		m_ToolbarExtender.Reset();
	}

	m_ToolbarExtender = MakeShareable(new FExtender);

	m_AbilityEditorToolbar->SetupToolbar(m_ToolbarExtender, SharedThis(this));

	if (GetCurrentMode() == FAblAbilityEditorModes::AbilityTimelineMode)
	{
		m_AbilityEditorToolbar->AddTimelineToolbar(m_ToolbarExtender, SharedThis(this));
	}

	AddToolbarExtender(m_ToolbarExtender);
}

void FAblAbilityEditor::BindCommands()
{
	const FAblAbilityEditorCommands& Commands = FAblAbilityEditorCommands::Get();

	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();

	UICommandList->MapAction(Commands.m_AddTask,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::AddNewTask));
	UICommandList->MapAction(Commands.m_RemoveTask,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::RemoveCurrentTask));
	UICommandList->MapAction(Commands.m_PlayAbility,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::PlayAbility));
	UICommandList->MapAction(Commands.m_StopAbility,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::StopAbility),
		FCanExecuteAction::CreateSP(this, &FAblAbilityEditor::IsPlayingAbility));
	UICommandList->MapAction(Commands.m_StepAbility,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::StepAbility));
	UICommandList->MapAction(Commands.m_StepAbilityBackwards,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::StepAbilityBackwards));
	UICommandList->MapAction(Commands.m_Resize,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::ResizeAbility));
	UICommandList->MapAction(Commands.m_Validate,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::Validate));
	UICommandList->MapAction(Commands.m_SetPreviewAsset,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::SetPreviewAsset));
	UICommandList->MapAction(Commands.m_ResetPreviewAsset,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::ResetPreviewActor));
	UICommandList->MapAction(Commands.m_ToggleCost,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::ToggleCostView),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FAblAbilityEditor::ShowTaskCostEstimate));
	UICommandList->MapAction(Commands.m_ToggleShowCollisionQueries,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::ToggleShowQueryVolumes),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FAblAbilityEditor::IsDrawingCollisionQueries));
	UICommandList->MapAction(Commands.m_CaptureThumbnail,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::CaptureThumbnail));
	UICommandList->MapAction(Commands.m_ToggleDrawArrowComponent,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::ToggleDrawArrowComponent),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FAblAbilityEditor::IsDrawingArrowComponent));
	UICommandList->MapAction(Commands.m_ToggleDrawCameraComponent,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::ToggleDrawCameraComponent),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FAblAbilityEditor::IsDrawingCameraComponent));
	UICommandList->MapAction(Commands.m_ToggleDrawCharacterCollision,
		FExecuteAction::CreateSP(this, &FAblAbilityEditor::ToggleDrawCharacterCollision),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FAblAbilityEditor::IsDrawingCharacterCollision));
}

void FAblAbilityEditor::SpawnPreviewActor()
{
	if (m_PreviewActor.IsValid())
	{
		m_PreviewActor->Destroy();
		m_PreviewActor.Reset();
	}

	UWorld* PreviewWorld = GetPreviewScene().GetWorld();

	CreatePreviewActor(PreviewWorld);
	if (m_PreviewActor.IsValid())
	{
		m_PreviewAbilityComponent = m_PreviewActor->FindComponentByClass<UAblAbilityComponent>();
	}	
}

void FAblAbilityEditor::ResetPreviewActor()
{
	SpawnPreviewActor();
}

void FAblAbilityEditor::PlayAbility()
{
	if (IsPlayingAbility())
	{
		// Just toggle our Pause.
		PauseAbility();
	}
	else if (m_PreviewAbilityComponent.IsValid())
	{
		if (UAblAbility* CurrentAbility = GetAbility())
		{
			// Make sure we save so our changes are reflected. Maybe prompt for this?
			if (CurrentAbility->GetOutermost()->IsDirty())
			{
				SaveAsset_Execute();
			}

			m_PreviewAbilityComponent->PlayAbilityFromEditor(CurrentAbility);
		}
	}
}

void FAblAbilityEditor::PauseAbility()
{
	if (IsPlayingAbility())
	{
		m_IsPaused = !m_IsPaused;
	}
}

void FAblAbilityEditor::StepAbility()
{
	if (!IsPaused())
	{
		PauseAbility();
	}

	if (m_Viewport.IsValid() && m_EditorSettings)
	{
		TSharedPtr<SAbilityEditorViewport> ViewportWidget = m_Viewport.Pin()->GetViewportWidget();
		if (ViewportWidget.IsValid())
		{
			TSharedPtr<FAbilityEditorViewportClient> ViewportClient = ViewportWidget->GetAbilityViewportClient();
			if (ViewportClient.IsValid())
			{
				ViewportClient->TickWorld(m_EditorSettings->GetAbilityTimeStepDelta());
			}
		}
	}
}

void FAblAbilityEditor::StepAbilityBackwards()
{
	if (!IsPaused())
	{
		PauseAbility();
	}

	const float NewTimeValue = FMath::Max(GetAbilityCurrentTime() - m_EditorSettings->GetAbilityTimeStepDelta(), 0.0f);
	SetAbilityPreviewTime(NewTimeValue);
}

void FAblAbilityEditor::StopAbility()
{
	if (m_PreviewAbilityComponent.IsValid())
	{
		m_PreviewAbilityComponent->CancelAbility(m_PreviewAbilityComponent->GetActiveAbility(), EAblAbilityTaskResult::Successful);
	}
}

void FAblAbilityEditor::ToggleCostView()
{
	m_DisplayTaskCost = !m_DisplayTaskCost;
}

void FAblAbilityEditor::ToggleShowQueryVolumes()
{
	FAblAbilityDebug::EnableDrawQueries(!FAblAbilityDebug::ShouldDrawQueries());
}

void FAblAbilityEditor::ToggleDrawCharacterCollision()
{
	m_DrawCharacterCollision = !m_DrawCharacterCollision;
	ResetPreviewActor();
}

void FAblAbilityEditor::ToggleDrawArrowComponent()
{
	m_DrawArrowComponent = !m_DrawArrowComponent;
	ResetPreviewActor();
}

void FAblAbilityEditor::ToggleDrawCameraComponent()
{
	m_DrawCameraComponent = !m_DrawCameraComponent;
	ResetPreviewActor();
}

void FAblAbilityEditor::CaptureThumbnail()
{
	if (m_Viewport.IsValid())
	{
		m_Viewport.Pin()->CaptureThumbnail();
	}
}

void FAblAbilityEditor::SpawnPreviewActorFromBlueprint(UWorld* World)
{
	UObject* SettingsPreviewAsset = GetDefault<UAblAbilityEditorSettings>(UAblAbilityEditorSettings::StaticClass())->m_PreviewAsset.TryLoad();

	if (SettingsPreviewAsset && SettingsPreviewAsset->IsA(UBlueprint::StaticClass()))
	{
		UBlueprint* Blueprint = CastChecked<UBlueprint>(SettingsPreviewAsset);
		if (Blueprint->IsA(UAnimBlueprint::StaticClass()) || !Blueprint->GeneratedClass)
		{
			return;
		}

		// Clean up any existing actors.
		if (m_PreviewActor.IsValid())
		{
			m_PreviewActor->Destroy();
			m_PreviewActor.Reset();
		}

		const AActor* SpawnActor = Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject());

		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.Name = MakeUniqueObjectName(World, SpawnActor->GetClass());
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ActorSpawnParameters.ObjectFlags = EObjectFlags::RF_Transient; // We don't want to save anything on this object.	
		FTransform EmptyTransform;
		
		AActor* NewActor = Cast<AActor>(World->SpawnActor(SpawnActor->GetClass(), &EmptyTransform, ActorSpawnParameters));
		
		UAblAbilityComponent* AbilityComponent = NewActor->FindComponentByClass<UAblAbilityComponent>();
		if (!AbilityComponent)
		{
			UE_LOG(LogAbleEditor, Warning, TEXT("Ability Component was not found on Blueprint [%s]. One has been added automatically, but you will need to update the blueprint if you want to execute Abilities."), *Blueprint->GetName());
			
			AbilityComponent = NewObject<UAblAbilityComponent>(NewActor);
			NewActor->AddOwnedComponent(AbilityComponent);
			AbilityComponent->RegisterComponent();
		}

		if (!m_DrawCameraComponent)
		{
			if (UCameraComponent* CameraComponent = NewActor->FindComponentByClass<UCameraComponent>())
			{
				// Hide the Camera Mesh.
				CameraComponent->SetCameraMesh(nullptr);
			}
		}

		TArray<UActorComponent*> PrimitiveComponents = NewActor->GetComponentsByClass(UPrimitiveComponent::StaticClass());

		UPrimitiveComponent* PrimitiveComponent = nullptr;
		for (UActorComponent* ActorComponent : PrimitiveComponents)
		{
			PrimitiveComponent = CastChecked<UPrimitiveComponent>(ActorComponent);

			// Ignore our meshes.
			if (PrimitiveComponent->IsA<USkeletalMeshComponent>() || PrimitiveComponent->IsA<UStaticMeshComponent>())
			{
				continue;
			}

			// Check if we should keep this component (rendering-wise)
			if (PrimitiveComponent->IsA<UArrowComponent>())
			{
				if (m_DrawArrowComponent)
				{
					continue;
				}
			}
			else
			{
				if (m_DrawCharacterCollision)
				{
					continue;
				}
			}

			// Remove it.
			PrimitiveComponent->DestroyComponent();
		}

		m_PreviewActor = NewActor;
		m_PreviewAbilityComponent = AbilityComponent;
	}
}

FString FAblAbilityEditor::GetDocumentationLink() const
{
	return FBlueprintEditor::GetDocumentationLink(); // todo: point this at the correct documentation
}

#undef LOCTEXT_NAMESPACE
