// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablCollisionSweepTask.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "AbleCorePrivate.h"
#include "ablSettings.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblCollisionSweepTaskScratchPad::UAblCollisionSweepTaskScratchPad()
	: SourceTransform(FTransform::Identity),
	AsyncHandle(),
	AsyncProcessed(false)
{

}

UAblCollisionSweepTaskScratchPad::~UAblCollisionSweepTaskScratchPad()
{

}

UAblCollisionSweepTask::UAblCollisionSweepTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_FireEvent(false),
	m_Name(NAME_None),
	m_CopyResultsToContext(false),
	m_AllowDuplicateEntries(false),
	m_TaskRealm(EAblAbilityTaskRealm::ClientAndServer)
{

}

UAblCollisionSweepTask::~UAblCollisionSweepTask()
{

}

void UAblCollisionSweepTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	if (m_SweepShape)
	{
		UAblCollisionSweepTaskScratchPad* ScratchPad = Cast<UAblCollisionSweepTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
		m_SweepShape->GetQueryTransform(Context, ScratchPad->SourceTransform);
	}
}

void UAblCollisionSweepTask::OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const
{
	Super::OnTaskTick(Context, deltaTime);

	UAblCollisionSweepTaskScratchPad* ScratchPad = Cast<UAblCollisionSweepTaskScratchPad>(Context->GetScratchPadForTask(this));
	check(ScratchPad);
	if (m_SweepShape && m_SweepShape->IsAsync() && 
		UAbleSettings::IsAsyncEnabled() && 
		UAblAbilityTask::IsDone(Context))
	{
		if (ScratchPad->AsyncHandle._Handle == 0)
		{
			// We're at the end of the our task, so we need to perform our sweep. If We're Async, we need to queue
			// the query up and then process it next frame.
			ScratchPad->AsyncHandle = m_SweepShape->DoAsyncSweep(Context, ScratchPad->SourceTransform);
		}
	}
}

void UAblCollisionSweepTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const
{
	Super::OnTaskEnd(Context, result);

	TArray<FAblQueryResult> OutResults;
	if (m_SweepShape)
	{
		UAblCollisionSweepTaskScratchPad* ScratchPad = Cast<UAblCollisionSweepTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);

		if (m_SweepShape->IsAsync() && UAbleSettings::IsAsyncEnabled())
		{
			m_SweepShape->GetAsyncResults(Context, ScratchPad->AsyncHandle, OutResults);
			ScratchPad->AsyncProcessed = true;
		}
		else
		{
			m_SweepShape->DoSweep(Context, ScratchPad->SourceTransform, OutResults);
		}
	}

#if !(UE_BUILD_SHIPPING)
	if (IsVerbose())
	{
		PrintVerbose(FString::Printf(TEXT("Sweep found %d results."), OutResults.Num()));
	}
#endif

	if (OutResults.Num())
	{
		for (const UAblCollisionFilter* CollisionFilter : m_Filters)
		{
			CollisionFilter->Filter(Context, OutResults);

#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Filter %s executed. Entries remaining: %d"), *CollisionFilter->GetName(), OutResults.Num()));
			}
#endif
		}

		if (OutResults.Num()) // Early out if we filtered everything out.
		{
			if (m_CopyResultsToContext)
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Copying %d results into Context."), OutResults.Num()));
				}
#endif
				CopyResultsToContext(OutResults, Context);
			}

			if (m_FireEvent)
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Firing Collision Event %s with %d results."), *m_Name.ToString(), OutResults.Num()));
				}
#endif
				Context->GetAbility()->OnCollisionEvent(Context.Get(), m_Name, OutResults);
			}
		}
	}
}

UAblAbilityTaskScratchPad* UAblCollisionSweepTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return NewObject<UAblCollisionSweepTaskScratchPad>(Context.Get());
}

TStatId UAblCollisionSweepTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblCollisionSweepTask, STATGROUP_Able);
}

void UAblCollisionSweepTask::CopyResultsToContext(const TArray<FAblQueryResult>& InResults, const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	if (UAblAbilityComponent* AbilityComponent = Context->GetSelfAbilityComponent())
	{
		TArray<TWeakObjectPtr<AActor>> AdditionTargets;
		AdditionTargets.Reserve(InResults.Num());
		for (const FAblQueryResult& Result : InResults)
		{
			AdditionTargets.Add(Result.Actor);
		}
		AbilityComponent->AddAdditionTargetsToContext(Context, AdditionTargets, m_AllowDuplicateEntries);
	}
}

bool UAblCollisionSweepTask::IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	if (m_SweepShape && m_SweepShape->IsAsync() && UAbleSettings::IsAsyncEnabled())
	{
		UAblCollisionSweepTaskScratchPad* ScratchPad = Cast<UAblCollisionSweepTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
		return ScratchPad->AsyncHandle._Handle != 0;
	}
	else
	{
		return UAblAbilityTask::IsDone(Context);
	}
}

#if WITH_EDITOR

FText UAblCollisionSweepTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblCollisionSweepFormat", "{0}: {1}");
	FString ShapeDescription = TEXT("<none>");
	if (m_SweepShape)
	{
		ShapeDescription = m_SweepShape->DescribeShape();
	}
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(ShapeDescription));
}

#endif

#undef LOCTEXT_NAMESPACE