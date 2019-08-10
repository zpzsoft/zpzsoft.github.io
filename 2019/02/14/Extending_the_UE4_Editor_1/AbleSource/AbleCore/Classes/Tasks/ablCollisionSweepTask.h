// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Engine/EngineTypes.h"

#include "Tasks/ablCollisionFilters.h"
#include "Tasks/ablCollisionSweepTypes.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"
#include "WorldCollision.h"

#include "ablCollisionSweepTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Scratchpad for our Sweep Task. */
UCLASS(Transient)
class UAblCollisionSweepTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblCollisionSweepTaskScratchPad();
	virtual ~UAblCollisionSweepTaskScratchPad();

	/* The Query Transform. */
	UPROPERTY(Transient)
	FTransform SourceTransform;

	/* Our Async Handle. */
	FTraceHandle AsyncHandle;

	/* Whether or not the Async query has been processed. */
	UPROPERTY(transient)
	bool AsyncProcessed;
};

UCLASS()
class ABLECORE_API UAblCollisionSweepTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblCollisionSweepTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionSweepTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* On Task Tick. */
	virtual void OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const override;
	
	/* End our Task. */
	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const override;

	/* Returns true if the Task is Async. */
	virtual bool IsAsyncFriendly() const override { return m_SweepShape ? m_SweepShape->IsAsync() && !m_FireEvent : false; }
	
	/* Returns true if the Task only lasts a single frame. */
	virtual bool IsSingleFrame() const override { return false; }

	/* Returns true if our Task needs its OnTick method called. */
	virtual bool NeedsTick() const override { return m_SweepShape ? m_SweepShape->IsAsync() : false; }
	
	/* Returns the Realm this Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return m_TaskRealm; }

	/* Returns true if our Task is completed. */
	virtual bool IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;

	/* Creates the Scratchpad for our Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;

	/* Returns the Profiler Stat ID for our Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category for this Task. */
	virtual FText GetTaskCategory() const { return LOCTEXT("AblCollisionSweepTaskCategory", "Collision"); }
	
	/* Returns the name of this Task. */
	virtual FText GetTaskName() const { return LOCTEXT("AblCollisionSweepTask", "Collision Sweep"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of this Task. */
	virtual FText GetTaskDescription() const { return LOCTEXT("AblCollisionSweepTaskDesc", "Performs a shape based sweep in the collision scene and returns any entities inside query. The Sweep is executed at the end of the task."); }
	
	/* Returns the color of this Task. */
	virtual FLinearColor GetTaskColor() const { return FLinearColor(0.0f, 1.0f, 0.5f); }
	
	/* Returns the estimated runtime cost of this Task. */
	virtual float GetEstimatedTaskCost() const { return UAblAbilityTask::GetEstimatedTaskCost() + ABLTASK_EST_COLLISION_SIMPLE_QUERY; }
	
	/* Returns true if the user is allowed to edit the Tasks realm. */
	virtual bool CanEditTaskRealm() const override { return true; }

	/* Returns the Fire Event Parameter. */
	FORCEINLINE bool GetFireEvent() const { return m_FireEvent; }

	/* Returns the Copy Results to Context Parameter. */
	FORCEINLINE bool GetCopyResultsToContext() const { return m_CopyResultsToContext; }

#endif
private:
	/* Helper method to copy our query results into the Ability Context. */
	void CopyResultsToContext(const TArray<FAblQueryResult>& InResults, const TWeakObjectPtr<const UAblAbilityContext>& Context) const;

protected:
	/* The shape for our sweep. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Sweep|Shape", meta = (DisplayName = "Sweep Shape"))
	UAblCollisionSweepShape* m_SweepShape;

	/* If true, we'll fire the OnCollisionEvent in the Ability Blueprint. */
	UPROPERTY(EditAnywhere, Category = "Sweep|Event", meta = (DisplayName = "Fire Event"))
	bool m_FireEvent;

	/* A String identifier you can use to identify this specific task in the ability blueprint. */
	UPROPERTY(EditAnywhere, Category = "Sweep|Event", meta = (DisplayName = "Name", EditCondition = m_FireEvent))
	FName m_Name;

	/* The Filters to execute on our results. */
	UPROPERTY(EditAnywhere, Instanced, Category = "Sweep|Filter", meta = (DisplayName = "Filters"))
	TArray<UAblCollisionFilter*> m_Filters;

	/* If true, the results of the query will be added to the Target Actor Array in the Ability Context. Note this takes 1 full frame to complete.*/
	UPROPERTY(EditAnywhere, Category = "Sweep|Misc", meta = (DisplayName = "Copy to Context"))
	bool m_CopyResultsToContext;

	/* If true, we won't check for already existing items when copying results to the context.*/
	UPROPERTY(EditAnywhere, Category = "Sweep|Misc", meta = (DisplayName = "Allow Duplicates", EditCondition = m_CopyResultsToContext))
	bool m_AllowDuplicateEntries;

	/* What realm, server or client, to execute this task. If your game isn't networked - this field is ignored. */
	UPROPERTY(EditAnywhere, Category = "Realm", meta = (DisplayName = "Realm"))
	TEnumAsByte<EAblAbilityTaskRealm> m_TaskRealm;
};

#undef LOCTEXT_NAMESPACE