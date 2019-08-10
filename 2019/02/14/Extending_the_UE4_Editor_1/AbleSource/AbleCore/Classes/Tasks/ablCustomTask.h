// Copyright (c) 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablCustomTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Scratchpad for our Task. */
UCLASS(Blueprintable, Transient)
class ABLECORE_API UAblCustomTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblCustomTaskScratchPad();
	virtual ~UAblCustomTaskScratchPad();
};


UCLASS(Abstract, Blueprintable, EditInlineNew, hidecategories = ("Optimization"))
class ABLECORE_API UAblCustomTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblCustomTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCustomTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;

	UFUNCTION(BlueprintNativeEvent, meta=(DisplayName="OnTaskStart"))
	void OnTaskStartBP(const UAblAbilityContext* Context) const;

	virtual void OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const override;

	UFUNCTION(BlueprintNativeEvent, meta=(DisplayName="OnTaskTick"))
	void OnTaskTickBP(const UAblAbilityContext* Context, float DeltaTime) const;

	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const override;

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "OnTaskEnd"))
	void OnTaskEndBP(const UAblAbilityContext* Context, const EAblAbilityTaskResult result) const;

	virtual bool IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "IsDone"))
	bool IsDoneBP(const UAblAbilityContext* Context) const;

	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const override;

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "CreateScratchPad"))
	UAblCustomTaskScratchPad* CreateScratchPadBP(UAblAbilityContext* Context) const;

	UFUNCTION(BlueprintPure, Category = "Able|Custom Task", meta = (DisplayName = "GetScratchPad"))
	UAblCustomTaskScratchPad* GetScratchPad(UAblAbilityContext* Context) const;

	/* Returns true if our Task is Async. */
	virtual bool IsAsyncFriendly() const override { return false; }

	/* Returns true if our Task only lasts for a single frame. */
	virtual bool IsSingleFrame() const override;

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "IsSingleFrame"))
	bool IsSingleFrameBP() const;

	/* Returns the Realm our Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override;

	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "GetTaskRealm"))
	EAblAbilityTaskRealm GetTaskRealmBP() const;

	// Various Helpers...
	UFUNCTION(BlueprintPure, Category = "Able|Custom Task", meta = (DisplayName = "GetActorsForTask"))
	void GetActorsForTaskBP(const UAblAbilityContext* Context, TArray<AActor*>& OutActorArray) const;

	/* Returns the Profiler Stat ID for our Task. */
	virtual TStatId GetStatId() const override;

	/* Returns the category of this Task. */
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "GetTaskCategory"))
	FText GetTaskCategoryBP() const;

	/* Returns the name of this Task. */
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "GetTaskName"))
	FText GetTaskNameBP() const;

	/* Returns the dynamic, descriptive name of our Task. */
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "GetDescriptiveTaskName"))
	FText GetDescriptiveTaskNameBP() const;

	/* Returns the description of this Task. */
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "GetTaskDescription"))
	FText GetTaskDescriptionBP() const;

	/* Returns the color of this Task. */
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "GetTaskColor"))
	FLinearColor GetTaskColorBP() const;

#if WITH_EDITOR
	/* Returns the category of this Task. */
	virtual FText GetTaskCategory() const override;

	/* Returns the name of this Task. */
	virtual FText GetTaskName() const override;

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;

	/* Returns the description of this Task. */
	virtual FText GetTaskDescription() const override;

	/* Returns the color of this Task. */
	virtual FLinearColor GetTaskColor() const override;

	/* Returns the estimated runtime cost of this Task. */
	virtual float GetEstimatedTaskCost() const override { return UAblAbilityTask::GetEstimatedTaskCost() + ABLTASK_EST_BP_EVENT; }

	/* Returns how to display the End time of our Task. */
	virtual EVisibility ShowEndTime() const { return EVisibility::Collapsed; }

	/* Returns true if the user is allowed to edit the Tasks realm. */
	virtual bool CanEditTaskRealm() const override { return true; }
#endif

};

#undef LOCTEXT_NAMESPACE