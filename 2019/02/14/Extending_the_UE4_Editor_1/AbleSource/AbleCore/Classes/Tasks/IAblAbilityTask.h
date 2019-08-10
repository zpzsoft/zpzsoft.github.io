// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "ablAbilityContext.h"
#include "Targeting/ablTargetingBase.h"
#include "Tasks/ablTaskWeights.inl"
#include "UObject/ObjectMacros.h"

#if WITH_EDITOR
#include "Runtime/SlateCore/Public/Layout/Visibility.h"
#endif

#include "IAblAbilityTask.generated.h"

class UAblAbilityComponent;

#define LOCTEXT_NAMESPACE "AbleCore"

#if WITH_EDITORONLY_DATA
// Event for other objects that need to watch for changes in the Task.
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAblAbilityTaskPropertyModified, class UAblAbilityTask& /*Task*/, struct FPropertyChangedEvent& /*PropertyChangedEvent*/);
#endif

UENUM(BlueprintType)
enum EAblAbilityTaskResult
{
	// Task finished normally (e.g. reached its end time).
	Successful UMETA(DisplayName="Successful"),

	// Ability is being branched to another ability.
	Branched UMETA(DisplayName="Branched"), 

	// Ability was interrupted.
	Interrupted UMETA(DisplayName="Interrupted") 
};

UENUM(BlueprintType)
enum EAblAbilityTaskRealm
{
	Client = 0 UMETA(DisplayName="Client"),
	Server UMETA(DisplayName="Server"),
	ClientAndServer UMETA(DisplayName="Client And Server"),

	TotalRealms UMETA(DisplayName="Internal_DO_NOT_USE")
};

/* Tasks are meant to be stateless, as you could have multiple actors referencing the same
 * ability, which are then executing the same tasks. To avoid copying the entire ability(or even just the tasks)
 * a Task can create a custom type of scratch pad and it'll automatically be instantiated and passed to the task so you can
 * keep any state changes you need there. 
 */

UCLASS(Abstract, Transient)
class ABLECORE_API UAblAbilityTaskScratchPad : public UObject
{
	GENERATED_BODY()
public:
	UAblAbilityTaskScratchPad() { };
	virtual ~UAblAbilityTaskScratchPad() { };
};


/* Tasks execute a single bit of logic. They are the building blocks of an ability.*/
UCLASS(Abstract, EditInlineNew, HideCategories="Internal")
class ABLECORE_API  UAblAbilityTask : public UObject
{
	GENERATED_BODY()

protected:
	UAblAbilityTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblAbilityTask();

public:
	/* UObject override to fix up any properties. */
	virtual void PostInitProperties() override;

	/* Called to determine is a Task can start. Default behavior is to simply check if our context time is > our start time. */
	virtual bool CanStart(const TWeakObjectPtr<const UAblAbilityContext>& Context, float CurrentTime, float DeltaTime) const;

	/* Called as soon as the task is started. Do any per-run initialization here.*/
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const;

	/* Called per tick if NeedsTick returns true. Not called on the 1st frame (OnTaskStart is called instead).*/
	virtual void OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const;

	/* Called to determine if a Task can end. Default behavior is to see if our context time is > than our end time. */
	virtual bool IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const;

	/* Called when a task is ending. The result tells us why the task is ending.*/
	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const;

	/* Return true if the task will be ticked each frame during its execution.*/
	virtual bool NeedsTick() const { return !IsSingleFrame(); }

	/* Returns the Duration of the Task.*/
	FORCEINLINE float GetDuration() const {	return GetEndTime() - GetStartTime();	}
	
	/* Return the Start time of the Task.*/
	FORCEINLINE virtual float GetStartTime() const { return m_StartTime; }
	
	/* Returns the End time of the Task.*/
	FORCEINLINE virtual float GetEndTime() const { return IsSingleFrame() ? GetStartTime() + KINDA_SMALL_NUMBER : m_EndTime; }
	
	/* Returns whether this Task only occurs during a Single Frame, or not. */
	FORCEINLINE virtual bool  IsSingleFrame() const { return false; }

	/* Returns whether this Task can be executed Asynchronously or not. */
	FORCEINLINE virtual bool  IsAsyncFriendly() const { return false; }

	/* Returns the Realm (Client/Server/Both) that this Task is allowed to execute on. */
	FORCEINLINE virtual EAblAbilityTaskRealm GetTaskRealm() const { return EAblAbilityTaskRealm::Client; }

	/* Returns whether this Task has any dependencies or not. */
	FORCEINLINE bool HasDependencies() const { return m_Dependencies.Num() != 0; }

	/* Returns any Tasks we are dependent on. */
	FORCEINLINE const TArray<const UAblAbilityTask*>& GetTaskDependencies() const { return m_Dependencies; }

	/* Returns whether to run in verbose mode or not. */
	FORCEINLINE bool IsVerbose() const { return m_Verbose; }
	
	/* Called when a Task is about to begin execution. Used to allocate any run-specific memory requirements. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const { return nullptr; }

	/* Returns the StatId for this Task, used by the Profiler. */
	virtual TStatId GetStatId() const { checkNoEntry(); return TStatId(); }

	bool IsValidForNetMode(ENetMode NetMode) const;

	/* Call back for when the time has changed unexpectedly (normally due to timeline scrubbing in the editor). */
	virtual void OnAbilityTimeSet(const TWeakObjectPtr<const UAblAbilityContext>& Context) { };

#if WITH_EDITOR
	/* Returns the Category for this task. Supports Subcategories using the | character (e.g. Main Character|Sub Category|etc). */
	virtual FText GetTaskCategory() const { return LOCTEXT("AblAbilityTaskCategory", "Misc"); }

	/* Returns the Tasks name. */
	virtual FText GetTaskName() const { return LOCTEXT("AblAbilityTask", "Unknown Task"); }

	/* Returns a more descriptive, dynamically constructed Task name. */
	virtual FText GetDescriptiveTaskName() const { return GetTaskName(); }

	/* Returns a description of what the Task does. */
	virtual FText GetTaskDescription() const { return FText::GetEmpty(); }

	/* The color to use to represent this Task in the Editor. */
	virtual FLinearColor GetTaskColor() const { return FLinearColor::White;  }
	
	/* Estimated Task Cost is used to give a rough estimate on how "expensive" the task, and thus the ability itself, is.
	 * The value is between 0.0 - 1.0 where 0.0 is "free" and 1.0 is computationally burdensome.
	 * Things to consider when decided on an estimate:
	 * - Does the task require Ticking? 
	 *    - Is it Async friendly? 
	 * - Does it fire a blueprint event? 
	 * - Does it do any type of collision query (is it a simple sphere query, or a more expensive raycast)? 
	 * 
	 * In the end, this is just a very rough guess to help your content creators. As always, profiling tools are your best bet for
	 * truly hunting down performance hot spots.
	 *
	 * Feel free to use some of the defines above as an easy way to estimate.*/
	virtual float GetEstimatedTaskCost() const;

	/* Sets the Start time of the Task. */
	void SetStartTime(float Time) { m_StartTime = Time; }

	/* Sets the End time of the Task. */
	void SetEndTime(float Time) { m_EndTime = Time; }

	/* The End Time of our Task can have various behaviors within the editor depending on our options. 
	  - Hidden = End Time is marked as Read Only in the Editor.
	  - Collapsed = End Time is not shown in the Editor.
	  - Visible = End Time is visible and editable in the Editor.*/
	virtual EVisibility ShowEndTime() const { return IsSingleFrame() ? EVisibility::Hidden : EVisibility::Visible; }

	/* Whether or not to allow the User to edit the Realm (Client/Server/Both) that a Task can execute in.*/
	virtual bool CanEditTaskRealm() const { return false; }

	/* UObject Override to fix up any Properties. */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/* Returns a modifiable version of our Task dependencies. */
	TArray<const UAblAbilityTask*>& GetMutableTaskDependencies() { return m_Dependencies; }

	/* Adds a Task dependency for this Task.*/
	void AddDependency(const UAblAbilityTask* Task);

	/* Removes a Task dependency for this Task.*/
	void RemoveDependency(const UAblAbilityTask* Task);

#if WITH_EDITORONLY_DATA
	/* Returns the On Task Property Modified delegate. */
	FOnAblAbilityTaskPropertyModified& GetOnTaskPropertyModified() { return m_OnTaskPropertyModified; }
#endif

#endif
protected:
	/* Populates the OutActorArray with all Context Targets relevant to this Task. */
	void GetActorsForTask(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<TWeakObjectPtr<AActor>>& OutActorArray) const;
	
	/* Returns only a Single Actor from a given Context Target, if Target Actors is used - only the first actor in the list is selected. */
	AActor* GetSingleActorFromTargetType(const TWeakObjectPtr<const UAblAbilityContext>& Context, EAblAbilityTargetType TargetType) const;

	/* Returns true (or false) if this Task is allowed to execute on this Actor based on the Task realm and Actor authority. */
	bool IsTaskValidForActor(const AActor* Actor) const;

#if !(UE_BUILD_SHIPPING)
	void PrintVerbose(const FString& Output) const;
#endif

	/* When the Task starts. */
	UPROPERTY(EditInstanceOnly, Category = "Timing", meta=(DisplayName = "Start Time", ClampMin = 0.0))
	float m_StartTime;

	/* When the Task ends. */
	UPROPERTY(EditInstanceOnly, Category = "Timing", meta=(DisplayName = "End Time", ClampMin = 0.001))
	float m_EndTime;

	/* Who you want this Task to affect.*/
	UPROPERTY(EditInstanceOnly, Category = "Targets", meta=(DisplayName = "Targets"))
	TArray<TEnumAsByte<EAblAbilityTargetType>> m_TaskTargets;

	/* Any Tasks that this Task requires to be completed before beginning. */
	UPROPERTY(EditInstanceOnly, Category = "Internal")
	TArray<const UAblAbilityTask*> m_Dependencies;

	/* If true, this task will print out various debug information as it executes. This is automatically disabled in shipping builds. */
	UPROPERTY(EditInstanceOnly, Category = "Debug", meta=(DisplayName = "Verbose"))
	bool m_Verbose;

#if WITH_EDITORONLY_DATA
	/* Delegate for Task Properties being modified. */
	FOnAblAbilityTaskPropertyModified m_OnTaskPropertyModified;
#endif
};

#undef LOCTEXT_NAMESPACE