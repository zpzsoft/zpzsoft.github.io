// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "ablAbilityInstance.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "GameFramework/Actor.h"
#include "Misc/ScopeLock.h"

struct FAblAbilityTaskIsDonePredicate
{
	FAblAbilityTaskIsDonePredicate(float InCurrentTime)
		: CurrentTime(InCurrentTime)
	{

	}

	float CurrentTime;

	bool operator()(const UAblAbilityTask* A) const
	{
		return CurrentTime >= A->GetEndTime();
	}
};

UAblAbilityInstance::UAblAbilityInstance(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer),
	m_CurrentIteration(0)
{

}

UAblAbilityInstance::~UAblAbilityInstance()
{

}

void UAblAbilityInstance::Initialize(UAblAbilityContext& AbilityContext)
{
	m_Ability = AbilityContext.GetAbility();
	m_Context = &AbilityContext;

	ENetMode NetMode = NM_Standalone;
	if (AbilityContext.GetSelfActor())
	{
		NetMode = AbilityContext.GetSelfActor()->GetNetMode();
	}

	// Sort our Tasks into Sync/Async queues.
	const TArray<UAblAbilityTask*> & Tasks = m_Ability->GetTasks();
	for (UAblAbilityTask* Task : Tasks)
	{
		if (Task->IsValidForNetMode(NetMode))
		{
			if (Task->IsAsyncFriendly())
			{
				m_AsyncTasks.Add(Task);
			}
			else
			{
				m_SyncTasks.Add(Task);
			}
		}
	}

	m_FinishedAyncTasks.Reserve(m_AsyncTasks.Num());
	m_FinishedSyncTasks.Reserve(m_SyncTasks.Num());

	// Populate our map of Tasks we need to keep track of due to other Tasks being dependent on them.
	const TArray<const UAblAbilityTask*>& TaskDependencies = m_Ability->GetAllTaskDependencies();
	for (const UAblAbilityTask* TaskDependency : TaskDependencies)
	{
		check(!m_TaskDependencyMap.Contains(TaskDependency));
		m_TaskDependencyMap.Add(TaskDependency);
	}

	// Tell our C++ Delegate
	if (m_Context->GetSelfAbilityComponent())
	{
		m_Context->GetSelfAbilityComponent()->GetOnAbilityStart().Broadcast(*m_Ability);
	}

	// Call our OnAbilityStart
	m_Ability->OnAbilityStart(m_Context);
}

void UAblAbilityInstance::PreUpdate()
{
	// Copy over any targets that should be added to our context.
	check(m_Context != nullptr);
	if (m_AdditionalTargets.Num())
	{
		m_Context->GetMutableTargetActors().Append(m_AdditionalTargets);
		m_AdditionalTargets.Empty();
	}

	// Check our Loop logic before we begin to update. 
	if (m_Ability->IsLooping())
	{
		// This is the start, end time of the Loop. X = Start, Y = End
		const FVector2D& LoopRange = m_Ability->GetLoopRange();
		if (m_Context->GetCurrentTime() > LoopRange.Y)
		{
			// We've completed an iteration, add it to our count and check if we've reached max iterations.
			++m_CurrentIteration;
			if (!IsDone())
			{
				InternalStopRunningTasks(EAblAbilityTaskResult::Successful, true);

				// Reset our time to the start of the loop range, and we'll keep going.
				m_Context->SetCurrentTime(LoopRange.X);

				m_FinishedAyncTasks.RemoveAll([&](const UAblAbilityTask* Task) 
				{
					return Task->GetStartTime() >= LoopRange.X && Task->GetEndTime() <= LoopRange.Y;
				});

				m_FinishedSyncTasks.RemoveAll([&](const UAblAbilityTask* Task) 
				{
					return Task->GetStartTime() >= LoopRange.X && Task->GetEndTime() <= LoopRange.Y;
				});
			}
			else
			{
				FinishAbility();
			}
		}
	}
}

uint32 UAblAbilityInstance::GetAbilityNameHash() const
{
	check(m_Ability != nullptr);
	return m_Ability->GetAbilityNameHash();
}

bool UAblAbilityInstance::HasAsyncTasks() const
{
	return m_AsyncTasks.Num() > 0 || m_ActiveAsyncTasks.Num() > 0;
}

bool UAblAbilityInstance::IsDone() const
{
	check(m_Ability != nullptr);
	if (m_Ability->IsLooping())
	{
		const uint32 MaxIterations = m_Ability->GetLoopMaxIterations();
		// Zero = infinite.
		return MaxIterations != 0 && m_CurrentIteration >= MaxIterations;
	}
	else
	{
		return GetCurrentTime() > m_Ability->GetLength();
	}
}

bool UAblAbilityInstance::IsChanneled() const
{
	return m_Ability != nullptr && m_Ability->IsChanneled();
}

bool UAblAbilityInstance::CheckChannelConditions() const
{
	check(m_Ability != nullptr);

	bool ConditionResult = false;
	for (const UAblChannelingBase* Condition : m_Ability->GetChannelConditions())
	{
		ConditionResult = Condition->GetConditionResult(*m_Context);

		if (ConditionResult && !m_Ability->MustPassAllChannelConditions())
		{
			// One condition passed.
			return true;
		}
		else if (!ConditionResult && m_Ability->MustPassAllChannelConditions())
		{
			return false;
		}
	}

	return ConditionResult;	
}

EAblAbilityTaskResult UAblAbilityInstance::GetChannelFailureResult() const
{
	check(m_Ability != nullptr);
	return m_Ability->GetChannelFailureResult();
}

void UAblAbilityInstance::ResetTime(bool ToLoopStart /*= false*/)
{
	check(m_Ability != nullptr);
	if (ToLoopStart && m_Ability->IsLooping())
	{
		// Reset to the start of our loop.
		m_Context->SetCurrentTime(m_Ability->GetLoopRange().X);
	}
	else
	{
		m_Context->SetCurrentTime(0.0f);
	}
}

void UAblAbilityInstance::AsyncUpdate(float CurrentTime, float DeltaTime)
{
	InternalUpdateTasks(m_AsyncTasks, m_ActiveAsyncTasks, m_FinishedAyncTasks, CurrentTime, DeltaTime);
}

void UAblAbilityInstance::SyncUpdate(float DeltaTime)
{
	const float CurrentTime = m_Context->GetCurrentTime();
	const float AdjustedTime = CurrentTime + DeltaTime;

	InternalUpdateTasks(m_SyncTasks, m_ActiveSyncTasks, m_FinishedSyncTasks, CurrentTime, DeltaTime);

	m_Context->UpdateTime(DeltaTime);
}

void UAblAbilityInstance::SetStackCount(int32 TotalStacks)
{
	check(m_Context != nullptr);
	m_Context->SetStackCount(TotalStacks);
}

int32 UAblAbilityInstance::GetStackCount() const
{
	check(m_Context != nullptr);
	return m_Context->GetCurrentStackCount();
}

void UAblAbilityInstance::AddAdditionalTargets(const TArray<TWeakObjectPtr<AActor>>& AdditionalTargets, bool AllowDuplicates /*= false*/)
{
	check(m_Context != nullptr);

	const TArray<TWeakObjectPtr<AActor>>& CurrentTargets = m_Context->GetTargetActorsWeakPtr();

	if (AllowDuplicates)
	{
		FScopeLock Lock(&m_AddTargetCS);
		// Simple Append.
		m_AdditionalTargets.Append(AdditionalTargets);
	}
	else
	{
		//Worst Case is O(n^2) which sucks, but hopefully our list is small.
		TArray<TWeakObjectPtr<AActor>> UniqueEntries;

		for (const TWeakObjectPtr<AActor>& Target : AdditionalTargets)
		{
			if (!CurrentTargets.Contains(Target))
			{
				UniqueEntries.AddUnique(Target);
			}
		}

		FScopeLock Lock(&m_AddTargetCS);
		m_AdditionalTargets.Append(UniqueEntries);
	}
}

void UAblAbilityInstance::StopAbility()
{
	InternalStopRunningTasks(EAblAbilityTaskResult::Successful);

	// Tell our C++ Delegate
	if (m_Context->GetSelfAbilityComponent())
	{
		m_Context->GetSelfAbilityComponent()->GetOnAbilityEnd().Broadcast(*m_Ability, EAblAbilityTaskResult::Successful);
	}

	// Call our Ability method
	m_Ability->OnAbilityEnd(m_Context);
}

void UAblAbilityInstance::FinishAbility()
{
	InternalStopRunningTasks(EAblAbilityTaskResult::Successful);

	// Tell our C++ Delegate
	if (m_Context->GetSelfAbilityComponent())
	{
		m_Context->GetSelfAbilityComponent()->GetOnAbilityEnd().Broadcast(*m_Ability, EAblAbilityTaskResult::Successful);
	}

	// Call our Ability method.
	m_Ability->OnAbilityEnd(m_Context);
}

void UAblAbilityInstance::InterruptAbility()
{
	InternalStopRunningTasks(EAblAbilityTaskResult::Interrupted);

	// Tell our C++ Delegate
	if (m_Context->GetSelfAbilityComponent())
	{
		m_Context->GetSelfAbilityComponent()->GetOnAbilityEnd().Broadcast(*m_Ability, EAblAbilityTaskResult::Interrupted);
	}

	// Call our Ability method.
	m_Ability->OnAbilityInterrupt(m_Context);
}

void UAblAbilityInstance::BranchAbility()
{
	InternalStopRunningTasks(EAblAbilityTaskResult::Branched);

	// Tell our C++ Delegate
	if (m_Context->GetSelfAbilityComponent())
	{
		m_Context->GetSelfAbilityComponent()->GetOnAbilityEnd().Broadcast(*m_Ability, EAblAbilityTaskResult::Branched);
	}

	// Call our Ability method.
	m_Ability->OnAbilityBranch(m_Context);
}

UAblAbilityContext& UAblAbilityInstance::GetMutableContext()
{
	verify(m_Context != nullptr)
	return *m_Context;
}

const UAblAbilityContext& UAblAbilityInstance::GetContext() const
{
	verify(m_Context != nullptr);
	return *m_Context;
}

const UAblAbility& UAblAbilityInstance::GetAbility() const
{
	verify(m_Ability != nullptr);
	return *m_Ability;
}

float UAblAbilityInstance::GetPlayRate() const
{
	return GetAbility().GetPlayRate(m_Context);
}

void UAblAbilityInstance::SetCurrentTime(float NewTime)
{
	m_Context->SetCurrentTime(NewTime);

	TArray<UAblAbilityTask*> NewTasks;
	TArray<UAblAbilityTask*> RemoveTasks;
	TArray<UAblAbilityTask*> CurrentTasks;

	const TArray<UAblAbilityTask*> & Tasks = m_Ability->GetTasks();
	for (UAblAbilityTask* Task : Tasks)
	{
		if (Task->CanStart(m_Context, NewTime, 0.0f ))
		{
			if (Task->IsAsyncFriendly() ? m_ActiveAsyncTasks.Contains(Task) : m_ActiveSyncTasks.Contains(Task))
			{
				CurrentTasks.Add(Task);
			}
			else
			{
				NewTasks.Add(Task);
			}
		}
	}

	// Now go through and Terminate any tasks that should no longer be running.
	for (int i = 0; i < m_ActiveSyncTasks.Num();)
	{
		if (!CurrentTasks.Contains(m_ActiveSyncTasks[i]))
		{
			m_ActiveSyncTasks[i]->OnTaskEnd(m_Context, Successful);
			m_ActiveSyncTasks.RemoveAt(i);
		}
		else
		{
			++i;
		}
	}

	for (int i = 0; i < m_ActiveAsyncTasks.Num();)
	{
		if (!CurrentTasks.Contains(m_ActiveAsyncTasks[i]))
		{
			m_ActiveAsyncTasks[i]->OnTaskEnd(m_Context, Successful);
			m_ActiveAsyncTasks.RemoveAt(i);
		}
		else
		{
			++i;
		}
	}

	// Start our New Tasks.
	for (UAblAbilityTask* Task : NewTasks)
	{
		Task->OnTaskStart(m_Context);

		if (Task->IsAsyncFriendly())
		{
			m_ActiveAsyncTasks.Add(Task);
		}
		else
		{
			m_ActiveSyncTasks.Add(Task);
		}
	}

	// Tell our Current Tasks to go ahead and do any logic they need to if the time was modified out from under them.
	for (UAblAbilityTask* Task : CurrentTasks)
	{
		Task->OnAbilityTimeSet(m_Context);
	}
}

void UAblAbilityInstance::InternalUpdateTasks(TArray<const UAblAbilityTask*>& InTasks, TArray<const UAblAbilityTask*>& InActiveTasks, TArray<const UAblAbilityTask*>& InFinishedTasks, float CurrentTime, float DeltaTime)
{
	const float AdjustedTime = CurrentTime + DeltaTime;
	const bool IsLooping = m_Ability->IsLooping();
	const FVector2D LoopTimeRange = m_Ability->GetLoopRange();

	// Just for readability...
	bool IsStarting = false;

	// if we need to clean up any active tasks.
	bool NeedArrayCleanUp = false;

	TArray<const UAblAbilityTask*> NewlyStartedTasks;
	TArray<const UAblAbilityTask*> FinishedDependentTasks;

	// First go through our Tasks and see if they need to be started.
	for (int i = 0; i < InTasks.Num(); )
	{
		const UAblAbilityTask* Task = InTasks[i];

		IsStarting = Task->CanStart(m_Context, CurrentTime, DeltaTime);

		if (IsLooping)
		{
			// If we're looping (And thus not cleaning up tasks as we complete them), make sure we haven't already finished this task.
			IsStarting = IsStarting && !InFinishedTasks.Contains(Task);
		}

		// Check any dependencies.
		if (IsStarting && Task->HasDependencies())
		{
			for (const UAblAbilityTask* DependantTask : Task->GetTaskDependencies())
			{
				
				// This *should* always be true, but in the editor sometimes the list isn't properly updated.
				// This is by far the assert I hit the most during iteration, so there's obviously a case I'm missing but I'd rather make this code a bit
				// more adaptive as it was written before I added the finished lists for both processes.
				if (!m_TaskDependencyMap.Contains(DependantTask))
				{
					FScopeLock DependencyMapLock(&m_DependencyMapCS);

					m_TaskDependencyMap.Add(DependantTask);
					m_TaskDependencyMap[DependantTask] = !DependantTask->IsAsyncFriendly() ? m_FinishedSyncTasks.Contains(DependantTask) : m_FinishedAyncTasks.Contains(DependantTask);
				}

				if (!m_TaskDependencyMap[DependantTask])
				{
					// A dependent Task is still executing, we can't start this frame.
					IsStarting = false;
					break;
				}
				else
				{
					// Our dependency is done, but we have some context targets that are pending. These could be needed, so delay for a frame.
					IsStarting = m_AdditionalTargets.Num() == 0;
				}
			}
		}

		if (IsStarting && !InActiveTasks.Contains(Task))
		{
			FScopeCycleCounter TaskScope(Task->GetStatId());

			// New Task to start.
			Task->OnTaskStart(m_Context);
			if (Task->IsSingleFrame())
			{
				// We can go ahead and end this task and forget about it.
				Task->OnTaskEnd(m_Context, EAblAbilityTaskResult::Successful);

				if (m_TaskDependencyMap.Contains(Task))
				{
					FScopeLock DependencyMapLock(&m_DependencyMapCS);
					m_TaskDependencyMap[Task] = true;
				}

				InFinishedTasks.Add(Task);
			}
			else
			{
				NewlyStartedTasks.Add(Task);
			}

			if (!IsLooping || Task->GetStartTime() < LoopTimeRange.X)
			{
				// If we aren't looping, or our task starts before our loop range, we can remove items from this list as we start tasks to cut down on future iterations.
				InTasks.RemoveAt(i, 1, false);
				continue;
			}
		}
		else if (AdjustedTime < Task->GetStartTime())
		{
			// Since our array is sorted by time. It's safe to early out.
			break;
		}

		++i;
	}

	InTasks.Shrink();

	// Update our actives.
	bool TaskCompleted = false;
	const UAblAbilityTask* ActiveTask = nullptr;
	for (int i = 0; i < InActiveTasks.Num(); )
	{
		ActiveTask = InActiveTasks[i];

		FScopeCycleCounter ActiveTaskScope(ActiveTask->GetStatId());

		TaskCompleted = ActiveTask->IsDone(m_Context);
		if (!TaskCompleted && ActiveTask->NeedsTick())
		{
			ActiveTask->OnTaskTick(m_Context, DeltaTime);
		}
		else if (TaskCompleted)
		{
			ActiveTask->OnTaskEnd(m_Context, EAblAbilityTaskResult::Successful);

			if (m_TaskDependencyMap.Contains(ActiveTask))
			{
				FScopeLock DependencyMapLock(&m_DependencyMapCS);
				m_TaskDependencyMap[ActiveTask] = true;
			}

			if (IsLooping)
			{
				InFinishedTasks.Add(ActiveTask);
			}

			InActiveTasks.RemoveAt(i, 1, false);
			continue;
		}

		++i;
	}

	// Move our newly started tasks over.
	InActiveTasks.Append(NewlyStartedTasks);

	InActiveTasks.Shrink();
}

void UAblAbilityInstance::InternalStopRunningTasks(EAblAbilityTaskResult Reason, bool ResetForLoop)
{
	if (ResetForLoop)
	{
		// We only want to remove Tasks that fall within our Loop time range.
		const FVector2D LoopRange = m_Ability->GetLoopRange();
		
		const UAblAbilityTask* CurrentTask = nullptr;

		for (int32 i = 0; i < m_ActiveAsyncTasks.Num();)
		{
			CurrentTask = m_ActiveAsyncTasks[i];
			if (CurrentTask->GetStartTime() >= LoopRange.X && CurrentTask->GetEndTime() <= LoopRange.Y)
			{
				CurrentTask->OnTaskEnd(m_Context, Reason);
				m_ActiveAsyncTasks.RemoveAt(i, 1, false);
				continue;
			}
			++i;
		}

		m_ActiveAsyncTasks.Shrink();

		for (int32 i = 0; i < m_ActiveSyncTasks.Num();)
		{
			CurrentTask = m_ActiveSyncTasks[i];
			if (CurrentTask->GetStartTime() >= LoopRange.X && CurrentTask->GetEndTime() <= LoopRange.Y)
			{
				CurrentTask->OnTaskEnd(m_Context, Reason);
				m_ActiveSyncTasks.RemoveAt(i, 1, false);
				continue;
			}
			++i;
		}

		m_ActiveSyncTasks.Shrink();

	}
	else
	{
		for (const UAblAbilityTask* Task : m_ActiveAsyncTasks)
		{
			Task->OnTaskEnd(m_Context, Reason);
		}
		m_ActiveAsyncTasks.Empty();

		for (const UAblAbilityTask* Task : m_ActiveSyncTasks)
		{
			Task->OnTaskEnd(m_Context, Reason);
		}
		m_ActiveSyncTasks.Empty();
	}

}

void UAblAbilityInstance::ResetTaskDependencyStatus()
{
	TMap<const UAblAbilityTask*, bool>::TIterator ResetIt = m_TaskDependencyMap.CreateIterator();
	const UAblAbilityTask* Task;
	for (; ResetIt; ++ResetIt)
	{
		Task = ResetIt.Key();
		if (!m_Ability->IsLooping())
		{
			ResetIt.Value() = false;
		}
		else
		{
			// Only reset if our task falls within our Loop range.
			if (Task->GetStartTime() > m_Ability->GetLoopRange().X)
			{
				ResetIt.Value() = false;
			}
		}
	}
}
