// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablAbilityInstance.generated.h"

class UAblAbility;
/* This class stores/controls all the variables needed during the execution of an Ability. 
 * It's not networked since the Context is the publicly exposed class and any variables that need to be kept
 * in sync with the Server/Queried by the user should be done there. This class is meant to be Fire & Forget. */
UCLASS(Transient)
class UAblAbilityInstance : public UObject
{
	GENERATED_BODY()
public:
	UAblAbilityInstance(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblAbilityInstance();

	/* Initializes this instance, allocates any scratch pads, sorts our tasks into Async/Sync arrays. */
	void Initialize(UAblAbilityContext& AbilityContext);

	/* Called just before we begin processing an update. There is no PostUpdate since we have no idea when our Async tasks have finished. */
	void PreUpdate();

	/* Returns our Ability's Name hash. */
	uint32 GetAbilityNameHash() const;

	/* True/False depending on if we have any Async friendly tasks. */
	bool HasAsyncTasks() const;

	/* True/False if we've gone past our Ability Length. Any remaining tasks will be told to finish. */
	bool IsDone() const;

	/* True/False if our Ability is channeled or not. */
	bool IsChanneled() const;
	
	/* Runs all the Ability Channel conditions and returns the result. */
	bool CheckChannelConditions() const;

	/* Returns how the user wants us to handle a failed channel. */
	EAblAbilityTaskResult GetChannelFailureResult() const;

	/* Resets our Time (optionally to the start of our loop range - if it exists). */
	void ResetTime(bool ToLoopStart = false);

	/* Async Update entry point - called by a Graph Task if allowed. */
	void AsyncUpdate(float CurrentTime, float DeltaTime);

	/* Synchronous update entry point. */
	void SyncUpdate(float DeltaTime);

	/* Sets this Ability's current stack count. */
	void SetStackCount(int32 TotalStacks);

	/* Returns this Ability's current stack count. */
	int32 GetStackCount() const;

	/* Adds Actors to be appended to the Target Actors in the Context before the next update. */
	void AddAdditionalTargets(const TArray<TWeakObjectPtr<AActor>>& AdditionalTargets, bool AllowDuplicates = false);

	// Ability State Modifiers.
	
	/* Stops the Ability and all currently running tasks are ended as successful before calling the Ability's OnAbilityFinished method. 
	 * No difference between Stop/FinishAbility yet. Stop is called via Blueprints or through the code API, while Finish is entirely internal. */
	void StopAbility();

	/* Stops the Ability and all currently running tasks are ended as successful before calling the Ability's OnAbilityFinished method. */
	void FinishAbility();

	/* Stops the Ability and all currently running tasks are ended as interrupted before calling the Ability's OnAbilityInterupt method. */
	void InterruptAbility();

	/* Stops the Ability and all currently running tasks are ended as branched before calling the Ability's OnAbilityBranch method. */
	void BranchAbility();

	// Accessors

	/* Returns a Mutable version of our Ability Context. */
	UAblAbilityContext& GetMutableContext();

	/* Returns a Non-mutable version of our Ability Context. */
	const UAblAbilityContext& GetContext() const;

	/* Returns our Ability. */
	const UAblAbility& GetAbility() const;

	/* Returns the current time of this Context. */
	FORCEINLINE float GetCurrentTime() const { return m_Context->GetCurrentTime(); }

	/* Returns the play rate of the Ability. */
	float GetPlayRate() const;

	/* Sets the current time of the Ability. */
	void SetCurrentTime(float NewTime);

protected:
	/* Shared code used by both Update versions (Async/Sync). */
	void InternalUpdateTasks(TArray<const UAblAbilityTask*>& InTasks, TArray<const UAblAbilityTask*>& InActiveTasks, TArray<const UAblAbilityTask*>& InFinishedTasks, float CurrentTime, float DeltaTime);
	
	/* Safely calls the appropriate OnTaskEnd for all running tasks. */
	void InternalStopRunningTasks(EAblAbilityTaskResult Reason, bool ResetForLoop = false);

	/* Sets all our Tasks we are keeping track of as being non-executed. */
	void ResetTaskDependencyStatus();

	/* What Iteration we are currently on. */
	UPROPERTY(Transient)
	uint32 m_CurrentIteration;

	/* Note we only store pointers to our tasks. The tasks themselves are stateless/purely functional (State is stored inside Scratch Pads as needed). */
	
	/* Array of all Asynchronous Tasks in this Ability. */
	UPROPERTY(Transient)
	TArray<const UAblAbilityTask*> m_AsyncTasks;

	/* Array of any Asynchronous Tasks currently being executed. */
	UPROPERTY(Transient)
	TArray<const UAblAbilityTask*> m_ActiveAsyncTasks;

	/* Array of Asynchronous Tasks that have been completed, used when looping. */
	UPROPERTY(Transient)
	TArray<const UAblAbilityTask*> m_FinishedAyncTasks;
	
	/* Array of all Synchronous Tasks in this Ability. */
	UPROPERTY(Transient)
	TArray<const UAblAbilityTask*> m_SyncTasks;

	/* Array of any Synchronous Tasks currently being executed. */
	UPROPERTY(Transient)
	TArray<const UAblAbilityTask*> m_ActiveSyncTasks;

	/* Array of Synchronous Tasks that have been completed, used when looping. */
	UPROPERTY(Transient)
	TArray<const UAblAbilityTask*> m_FinishedSyncTasks;

	/* The Ability. */
	UPROPERTY(Transient)
	const UAblAbility* m_Ability;

	/* The Ability Context. */
	UPROPERTY(Transient)
	UAblAbilityContext* m_Context;

	/* Targets to be added to our Context at the start of the next frame. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> m_AdditionalTargets;

	/* Tasks we need to keep track of, for dependency purposes. */
	UPROPERTY(Transient)
	TMap<const UAblAbilityTask*, bool> m_TaskDependencyMap;

	/* Critical Section for AddAdditionalTargets. */
	FCriticalSection m_AddTargetCS;

	/* Critical Section for Task Dependency Map. */
	FCriticalSection m_DependencyMapCS;
};