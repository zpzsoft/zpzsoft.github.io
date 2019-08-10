// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablPlayAbilityTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UCLASS(EditInlineNew)
class UAblPlayAbilityTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblPlayAbilityTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblPlayAbilityTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;

	/* Returns true if our Task only lasts for a single frame. */
	virtual bool IsSingleFrame() const override { return true; }

	/* Returns true if our Task is Async. */
	virtual bool IsAsyncFriendly() const override { return false; }
	
	/* Returns the realm our Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return EAblAbilityTaskRealm::Server; }

	/* Returns the Profiler Stat ID for our Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category for our Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblPlayAbilityCategory", "Gameplay"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblPlayAbilityTask", "Play Ability"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblPlayAbilityTaskDesc", "Creates an Ability Context and calls Activate Ability on the Target's Ability Component. This will cause an interrupt if valid."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(1.0f, 1.0f, 0.0f); } // Yellow/Gold

	/* Returns how to display the End time of our Task. */
	virtual EVisibility ShowEndTime() const { return EVisibility::Collapsed; }
#endif
protected:
	/* The Ability to Play. */
	UPROPERTY(EditAnywhere, Category = "Ability", meta = (DisplayName = "Ability"))
	TSubclassOf<UAblAbility> m_Ability;

	/* Who to set as the "Source" of this damage. */
	UPROPERTY(EditAnywhere, Category = "Ability", meta = (DisplayName = "Owner"))
	TEnumAsByte<EAblAbilityTargetType> m_Owner;

	/* Who to set as the "Source" of this damage. */
	UPROPERTY(EditAnywhere, Category = "Ability", meta = (DisplayName = "Instigator"))
	TEnumAsByte<EAblAbilityTargetType> m_Instigator;
};

#undef LOCTEXT_NAMESPACE
