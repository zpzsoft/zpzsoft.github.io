// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablBranchTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

class UAblBranchCondition;
class UInputSettings;

UCLASS(Transient)
class UAblBranchTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblBranchTaskScratchPad();
	virtual ~UAblBranchTaskScratchPad();

	/* Cached for the Custom Branch Conditional*/
	UPROPERTY(transient)
	const UAblAbility* BranchAbility;

	/* Keys to check for the Input Conditional */
	UPROPERTY(transient)
	TArray<struct FKey> CachedKeys;
};

UCLASS(EditInlineNew, hidecategories = ("Targets"))
class UAblBranchTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblBranchTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblBranchTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* On Task Tick*/
	virtual void OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const override;

	/* Returns true if our Task is Async supported. */
	virtual bool IsAsyncFriendly() const override { return false; }
	
	/* Returns true if this task needs its Tick method called. */
	virtual bool NeedsTick() const override { return true; }

	/* Returns the Realm to execute this task in. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return EAblAbilityTaskRealm::ClientAndServer; } // Client for Auth client, Server for AIs/Proxies.

	/* Create the Scratchpad for this Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;

	/* Return the Profiler Stat Id for this Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category for this Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblBranchCategory", "Logic"); }
	
	/* Returns the name of the Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblBranchTask", "Branch"); }
	
	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of the Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblBranchTaskDesc", "Allows this ability to immediately branch into another ability if the branch condition passes."); }
	
	/* Returns what color to use for this Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(1.0f, 0.0f, 1.0f); } //Purple
	
	/* Returns the estimated runtime cost of this Task. */
	virtual float GetEstimatedTaskCost() const override { return UAblAbilityTask::GetEstimatedTaskCost() + ABLTASK_EST_BP_EVENT; } // Assume worst case and they are using a BP Custom condition.
#endif
protected:
	/* Helper method to check our conditions. */
	bool CheckBranchCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context) const;

	/* The Ability to Branch to. */
	UPROPERTY(EditAnywhere, Category = "Branch", meta = (DisplayName = "Ability"))
	TSubclassOf<UAblAbility> m_BranchAbility;

	// If true, OnGetBranchAbility function will be called on the Ability itself and use whatever Ability it returns as the Branch Ability.
	UPROPERTY(EditAnywhere, Category = "Branch", meta = (DisplayName = "Dynamic Branch Ability"))
	bool m_DynamicBranchAbility;

	// If using Dynamic Branch Ability, this name will be passed along when calling the function (optional).
	UPROPERTY(EditAnywhere, Category = "Branch", meta = (DisplayName = "Dynamic Branch Event Name", EditCondition="m_DynamicBranchAbility"))
	FName m_DynamicBranchEventName;

	/* The Conditions for the Ability to Branch. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Branch", meta = (DisplayName = "Conditions"))
	TArray<UAblBranchCondition*> m_Conditions;

	// If there are multiple conditions this will require all to succeed, otherwise we stop at the first one that succeeds.
	UPROPERTY(EditAnywhere, Category = "Branch", meta = (DisplayName = "Must Pass All Conditions"))
	bool m_MustPassAllConditions;

	// If true, you're existing targets will be carried over to the branched Ability.
	UPROPERTY(EditAnywhere, Category = "Branch", meta = (DisplayName = "Copy Targets on Branch"))
	bool m_CopyTargetsOnBranch;


};

#undef LOCTEXT_NAMESPACE