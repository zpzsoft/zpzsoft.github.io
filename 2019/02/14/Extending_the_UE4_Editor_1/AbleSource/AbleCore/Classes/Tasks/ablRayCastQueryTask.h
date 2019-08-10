// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"
#include "WorldCollision.h"

#include "ablRayCastQueryTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Scratchpad for our Task. */
UCLASS(Transient)
class UAblRayCastQueryTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblRayCastQueryTaskScratchPad();
	virtual ~UAblRayCastQueryTaskScratchPad();

	/* Async Handle for our Query. */
	FTraceHandle AsyncHandle;

	/* Whether or not our Async query has been processed. */
	UPROPERTY(transient)
	bool AsyncProcessed;
};

UCLASS()
class ABLECORE_API UAblRayCastQueryTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblRayCastQueryTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblRayCastQueryTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* On Task Tick. */
	virtual void OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const override;

	/* Returns true if our Tick method needs to be called. */
	FORCEINLINE bool NeedsTick() const override { return m_UseAsyncQuery; }

	/* Returns true if our Task is Async. */
	FORCEINLINE bool IsAsyncFriendly() const override { return m_UseAsyncQuery && !m_FireEvent; } 
	
	/* Returns true if our Task only lasts for a single frame. */
	virtual bool IsSingleFrame() const override { return !m_UseAsyncQuery; }

	/* Returns the realm our Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return m_TaskRealm; } // Client for Auth client, Server for AIs/Proxies.

	/* The End time of our Task. */
	virtual float GetEndTime() const override { return GetStartTime() + 0.05f; }

	/* Returns true if our Task is done and ready for clean up. */
	virtual bool IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;

	/* Creates the Scratchpad for this Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;
	
	/* Returns the Profiler Stat ID for this Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category of our Task. */
	virtual FText GetTaskCategory() const { return LOCTEXT("AblRayCastQueryTaskCategory", "Collision"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const { return LOCTEXT("AblRayCastQueryTask", "Raycast Query"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const { return LOCTEXT("AblRayCastQueryTaskDesc", "Performs a raycast query in the collision scene."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const { return FLinearColor(1.0f, 0.0f, 1.0f); } // Bright Pink-ish
	
	/* Returns the estimated runtime cost of our Task. */
	virtual float GetEstimatedTaskCost() const { return UAblAbilityTask::GetEstimatedTaskCost() + ABLTASK_EST_COLLISION_QUERY_RAYCAST; }

	/* Returns how to display the End time of our Task. */
	virtual EVisibility ShowEndTime() const override { return EVisibility::Collapsed; }
	
	/* Returns true if the user is allowed to edit the realm this Task belongs to. */
	virtual bool CanEditTaskRealm() const override { return true; }

	/* Returns the Fire Event Parameter. */
	FORCEINLINE bool GetFireEvent() const { return m_FireEvent; }

	/* Returns the Copy Results to Context Parameter. */
	FORCEINLINE bool GetCopyResultsToContext() const { return m_CopyResultsToContext; }

#endif
private:
	/* Helper method to copy the results of our query into the Ability Context. */
	void CopyResultsToContext(const TArray<FHitResult>& InResults, const TWeakObjectPtr<const UAblAbilityContext>& Context) const;

protected:
	/* The length of the Ray. */
	UPROPERTY(EditAnywhere, Category = "Raycast", meta = (DisplayName = "Length"))
	float m_Length;

	/* If true, only return the blocking hit. Otherwise return all hits, including the blocking hit.*/
	UPROPERTY(EditAnywhere, Category = "Raycast", meta = (DisplayName = "Only Return Blocking Hit"))
	bool m_OnlyReturnBlockingHit;

	/* Where to start the Query. */
	UPROPERTY(EditAnywhere, Category="Raycast", meta =(DisplayName = "Location"))
	FAblAbilityTargetTypeLocation m_QueryLocation;

	/* The collision channel to use when we perform the query. */
	UPROPERTY(EditAnywhere, Category = "Raycast", meta = (DisplayName = "Collision Channel"))
	TEnumAsByte<ECollisionChannel> m_CollisionChannel;

	/* If true, we'll fire the OnRaycastEvent in the Ability Blueprint. */
	UPROPERTY(EditAnywhere, Category = "Raycast|Event", meta = (DisplayName = "Fire Event"))
	bool m_FireEvent;

	/* An (optional) Name for this query. Useful if you have multiple Raycasts in a single ability. */
	UPROPERTY(EditAnywhere, Category = "Raycast|Event", meta = (DisplayName = "Name", EditCondition = m_FireEvent))
	FName m_Name;

	/* If true, the results of the query will be added to the Target Actor Array in the Ability Context. Note this takes 1 full frame to complete.*/
	UPROPERTY(EditAnywhere, Category = "Raycast|Misc", meta = (DisplayName = "Copy to Context"))
	bool m_CopyResultsToContext;

	/* If true, we won't check for already existing items when copying results to the context.*/
	UPROPERTY(EditAnywhere, Category = "Raycast|Misc", meta = (DisplayName = "Allow Duplicates", EditCondition = m_CopyResultsToContext))
	bool m_AllowDuplicateEntries;

	/* What realm, server or client, to execute this task. If your game isn't networked - this field is ignored. */
	UPROPERTY(EditAnywhere, Category = "Realm", meta =(DisplayName ="Realm"))
	TEnumAsByte<EAblAbilityTaskRealm> m_TaskRealm;

	/* If true, the query is placed in the Async queue and the results are retrieved next frame. */
	UPROPERTY(EditAnywhere, Category = "Optimization", meta = (DisplayName = "Use Async Query"))
	bool m_UseAsyncQuery;
	
};

#undef LOCTEXT_NAMESPACE