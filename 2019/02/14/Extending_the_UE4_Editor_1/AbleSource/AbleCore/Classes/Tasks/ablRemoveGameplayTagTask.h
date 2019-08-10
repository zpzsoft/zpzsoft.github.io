// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Tasks/IAblAbilityTask.h"
#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
#include "UObject/ObjectMacros.h"
#include "ablRemoveGameplayTagTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UCLASS()
class ABLECORE_API UAblRemoveGameplayTagTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblRemoveGameplayTagTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblRemoveGameplayTagTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;

	/* Returns true if our Task lasts for only a single frame. */
	virtual bool IsSingleFrame() const { return true; }

	/* Returns the realm this Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return EAblAbilityTaskRealm::Server; }

	/* Returns the Profiler Stat ID for this Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category of our Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblRemoveGameplayTaskCategory", "Tags"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblRemoveGameplayTagTask", "Remove Gameplay Tag"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblRemoveGameplayTagDesc", "Removes the supplied Gameplay Tags on the task targets."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(1.0, 0.64, 0.0f); } // Orange

	/* How to display the End time of our Task. */
	virtual EVisibility ShowEndTime() const override { return EVisibility::Collapsed; }
#endif
protected:
	/* The Tags to Remove. */
	UPROPERTY(EditAnywhere, Category = "Tags", meta = (DisplayName = "Tag List"))
	TArray<FGameplayTag> m_TagList;

};

#undef LOCTEXT_NAMESPACE