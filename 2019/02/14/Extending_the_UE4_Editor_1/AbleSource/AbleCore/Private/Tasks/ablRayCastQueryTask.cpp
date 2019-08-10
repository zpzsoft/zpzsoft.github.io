// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablRayCastQueryTask.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "ablAbilityDebug.h"
#include "AbleCorePrivate.h"
#include "ablSettings.h"
#include "Components/SkeletalMeshComponent.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblRayCastQueryTaskScratchPad::UAblRayCastQueryTaskScratchPad()
	: AsyncHandle(),
	AsyncProcessed(false)
{

}

UAblRayCastQueryTaskScratchPad::~UAblRayCastQueryTaskScratchPad()
{

}

UAblRayCastQueryTask::UAblRayCastQueryTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Length(100.0f),
	m_OnlyReturnBlockingHit(false),
	m_QueryLocation(),
	m_CollisionChannel(ECC_WorldDynamic),
	m_FireEvent(false),
	m_Name(NAME_None),
	m_CopyResultsToContext(false),
	m_AllowDuplicateEntries(false),
	m_TaskRealm(EAblAbilityTaskRealm::Server),
	m_UseAsyncQuery(false)
{

}

UAblRayCastQueryTask::~UAblRayCastQueryTask()
{

}

void UAblRayCastQueryTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	FTransform QueryTransform;
	m_QueryLocation.GetTransform(*Context.Get(), QueryTransform);

	const FVector RayStart = QueryTransform.GetLocation();
	const FVector RayEnd = RayStart + QueryTransform.GetRotation().GetForwardVector() * m_Length;

	if (m_UseAsyncQuery && UAbleSettings::IsAsyncEnabled())
	{
		UAblRayCastQueryTaskScratchPad* ScratchPad = Cast<UAblRayCastQueryTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
		if (m_OnlyReturnBlockingHit)
		{
			ScratchPad->AsyncHandle = World->AsyncLineTraceByChannel(EAsyncTraceType::Single, RayStart, RayEnd, m_CollisionChannel);
		}
		else
		{
			ScratchPad->AsyncHandle = World->AsyncLineTraceByChannel(EAsyncTraceType::Multi, RayStart, RayEnd, m_CollisionChannel);
		}
	}
	else
	{
		TArray<FHitResult> HitResults;
		FHitResult TraceResult;
		if (m_OnlyReturnBlockingHit)
		{
			if (World->LineTraceSingleByChannel(TraceResult, RayStart, RayEnd, m_CollisionChannel))
			{
				HitResults.Add(TraceResult);
			}
		}
		else
		{
			World->LineTraceMultiByChannel(HitResults, RayStart, RayEnd, m_CollisionChannel);		
		}

#if !(UE_BUILD_SHIPPING)
		if (IsVerbose())
		{
			PrintVerbose(FString::Printf(TEXT("Raycast found %d results."), HitResults.Num()));
		}
#endif

		if (HitResults.Num())
		{
#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				// Quick distance print help to see if we hit ourselves.
				float DistanceToBlocker = HitResults[HitResults.Num() - 1].Distance;
				PrintVerbose(FString::Printf(TEXT("Raycast blocking hit distance: %4.2f."), DistanceToBlocker));
			}
#endif

			if (m_CopyResultsToContext)
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Copying %d results into Context."), HitResults.Num()));
				}
#endif
				CopyResultsToContext(HitResults, Context);
			}

			if (m_FireEvent)
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Firing Raycast Event %s with %d results."), *m_Name.ToString(), HitResults.Num()));
				}
#endif
				Context->GetAbility()->OnRaycastEvent(Context.Get(), m_Name, HitResults);
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawRaycastQuery(World, QueryTransform, m_Length);
	}
#endif
}

void UAblRayCastQueryTask::OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const
{
	Super::OnTaskTick(Context, deltaTime);

	if (IsAsyncFriendly() && UAbleSettings::IsAsyncEnabled())
	{
		UAblRayCastQueryTaskScratchPad* ScratchPad = Cast<UAblRayCastQueryTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
		if (!ScratchPad->AsyncProcessed && ScratchPad->AsyncHandle._Handle != 0)
		{
			AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
			check(SourceActor);

			UWorld* World = SourceActor->GetWorld();

			FTraceDatum Datum;
			if (World->QueryTraceData(ScratchPad->AsyncHandle, Datum))
			{
				if (m_CopyResultsToContext)
				{
					CopyResultsToContext(Datum.OutHits, Context);
				}

				if (m_FireEvent)
				{
					Context->GetAbility()->OnRaycastEvent(Context.Get(), m_Name, Datum.OutHits);
				}

				ScratchPad->AsyncProcessed = true;
			}

		}
	}
}

bool UAblRayCastQueryTask::IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	if (IsAsyncFriendly() && UAbleSettings::IsAsyncEnabled())
	{
		UAblRayCastQueryTaskScratchPad* ScratchPad = Cast<UAblRayCastQueryTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
		return ScratchPad->AsyncProcessed;
	}
	else
	{
		return UAblAbilityTask::IsDone(Context);
	}
}

UAblAbilityTaskScratchPad* UAblRayCastQueryTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return (IsAsyncFriendly() && UAbleSettings::IsAsyncEnabled()) ? NewObject<UAblRayCastQueryTaskScratchPad>(Context.Get()) : nullptr;
}

TStatId UAblRayCastQueryTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblRayCastQueryTask, STATGROUP_Able);
}

void UAblRayCastQueryTask::CopyResultsToContext(const TArray<FHitResult>& InResults, const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	if (UAblAbilityComponent* AbilityComponent = Context->GetSelfAbilityComponent())
	{
		TArray<TWeakObjectPtr<AActor>> AdditionTargets;
		AdditionTargets.Reserve(InResults.Num());
		for (const FHitResult& Result : InResults)
		{
			AdditionTargets.Add(Result.GetActor());
		}

		AbilityComponent->AddAdditionTargetsToContext(Context, AdditionTargets, m_AllowDuplicateEntries);
	}
}

#if WITH_EDITOR

FText UAblRayCastQueryTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblRayCastQueryFormat", "{0}: {1}");
	FString ShapeDescription = FString::Printf(TEXT("%.1fm"), m_Length * 0.01f);
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(ShapeDescription));
}

#endif

#undef LOCTEXT_NAMESPACE