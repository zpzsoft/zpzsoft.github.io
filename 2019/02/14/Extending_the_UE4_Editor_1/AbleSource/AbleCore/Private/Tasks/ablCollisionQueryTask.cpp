// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablCollisionQueryTask.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "AbleCorePrivate.h"
#include "ablSettings.h"

#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblCollisionQueryTaskScratchPad::UAblCollisionQueryTaskScratchPad()
	: AsyncHandle(),
	AsyncProcessed(false)
{

}

UAblCollisionQueryTaskScratchPad::~UAblCollisionQueryTaskScratchPad()
{

}

UAblCollisionQueryTask::UAblCollisionQueryTask(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer),
	m_FireEvent(false),
	m_Name(NAME_None),
	m_CopyResultsToContext(false),
	m_AllowDuplicateEntries(false),
	m_TaskRealm(EAblAbilityTaskRealm::ClientAndServer)
{

}

UAblCollisionQueryTask::~UAblCollisionQueryTask()
{

}

void UAblCollisionQueryTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	if (m_QueryShape)
	{
		if (m_QueryShape->IsAsync() && UAbleSettings::IsAsyncEnabled())
		{
			UAblCollisionQueryTaskScratchPad* ScratchPad = Cast<UAblCollisionQueryTaskScratchPad>(Context->GetScratchPadForTask(this));
			check(ScratchPad);
			ScratchPad->AsyncHandle = m_QueryShape->DoAsyncQuery(Context, ScratchPad->QueryTransform);
		}
		else
		{
			TArray<FAblQueryResult> Results;
			m_QueryShape->DoQuery(Context, Results);

#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Query found %d results."), Results.Num()));
			}
#endif

			if (Results.Num())
			{
				for (const UAblCollisionFilter* CollisionFilter : m_Filters)
				{
					CollisionFilter->Filter(Context, Results);

#if !(UE_BUILD_SHIPPING)
					if (IsVerbose())
					{
						PrintVerbose(FString::Printf(TEXT("Filter %s executed. Entries remaining: %d"), *CollisionFilter->GetName(), Results.Num()));
					}
#endif
				}

				// We could have filtered out all our entries, so check again if the array is empty.
				if (Results.Num())
				{
					if (m_CopyResultsToContext)
					{

#if !(UE_BUILD_SHIPPING)
						if (IsVerbose())
						{
							PrintVerbose(FString::Printf(TEXT("Copying %d results into Context."), Results.Num()));
						}
#endif

						CopyResultsToContext(Results, Context);
					}

					if (m_FireEvent)
					{

#if !(UE_BUILD_SHIPPING)
						if (IsVerbose())
						{
							PrintVerbose(FString::Printf(TEXT("Firing Collision Event %s with %d results."), *m_Name.ToString(), Results.Num()));
						}
#endif

						Context->GetAbility()->OnCollisionEvent(Context.Get(), m_Name, Results);
					}
				}

			}

		}
	}
}

void UAblCollisionQueryTask::OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const
{
	Super::OnTaskTick(Context, deltaTime);

	if (m_QueryShape && m_QueryShape->IsAsync() && UAbleSettings::IsAsyncEnabled())
	{
		UWorld* QueryWorld = m_QueryShape->GetQueryWorld(Context);
		check(QueryWorld);

		UAblCollisionQueryTaskScratchPad* ScratchPad = Cast<UAblCollisionQueryTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);

		if (!ScratchPad->AsyncProcessed && QueryWorld->IsTraceHandleValid(ScratchPad->AsyncHandle, true))
		{
			FOverlapDatum Datum;
			if (QueryWorld->QueryOverlapData(ScratchPad->AsyncHandle, Datum))
			{
				TArray<FAblQueryResult> Results;
				m_QueryShape->ProcessAsyncOverlaps(Context, ScratchPad->QueryTransform, Datum.OutOverlaps, Results);

#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Query found %d results."), Results.Num()));
				}
#endif

				if (Results.Num())
				{
					for (const UAblCollisionFilter* CollisionFilter : m_Filters)
					{
						CollisionFilter->Filter(Context, Results);
#if !(UE_BUILD_SHIPPING)
						if (IsVerbose())
						{
							PrintVerbose(FString::Printf(TEXT("Filter %s executed. Entries remaining: %d"), *CollisionFilter->GetName(), Results.Num()));
						}
#endif
					}

					if (Results.Num())
					{
						if (m_CopyResultsToContext)
						{
#if !(UE_BUILD_SHIPPING)
							if (IsVerbose())
							{
								PrintVerbose(FString::Printf(TEXT("Copying %d results into Context."), Results.Num()));
							}
#endif
							CopyResultsToContext(Results, Context);
						}

						if (m_FireEvent)
						{
#if !(UE_BUILD_SHIPPING)
							if (IsVerbose())
							{
								PrintVerbose(FString::Printf(TEXT("Firing Collision Event %s with %d results."), *m_Name.ToString(), Results.Num()));
							}
#endif
							Context->GetAbility()->OnCollisionEvent(Context.Get(), m_Name, Results);
						}
					}
				}

				ScratchPad->AsyncProcessed = true;
			}
		}
	}
}

bool UAblCollisionQueryTask::IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	if (m_QueryShape && m_QueryShape->IsAsync() && UAbleSettings::IsAsyncEnabled())
	{
		UAblCollisionQueryTaskScratchPad* ScratchPad = Cast<UAblCollisionQueryTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
		return ScratchPad->AsyncProcessed;
	}
	else
	{
		return UAblAbilityTask::IsDone(Context);
	}
}

UAblAbilityTaskScratchPad* UAblCollisionQueryTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return (m_QueryShape && m_QueryShape->IsAsync()) ? NewObject<UAblCollisionQueryTaskScratchPad>(Context.Get()) : nullptr;
}

TStatId UAblCollisionQueryTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblCollisionQueryTask, STATGROUP_Able);
}

void UAblCollisionQueryTask::CopyResultsToContext(const TArray<FAblQueryResult>& InResults, const TWeakObjectPtr<const UAblAbilityContext>& Context) const
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

#if WITH_EDITOR

FText UAblCollisionQueryTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblCollisionQueryFormat", "{0}: {1}");
	FString ShapeDescription = TEXT("<none>");
	if (m_QueryShape)
	{
		ShapeDescription = m_QueryShape->DescribeShape();
	}
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(ShapeDescription));
}

#endif

#undef LOCTEXT_NAMESPACE