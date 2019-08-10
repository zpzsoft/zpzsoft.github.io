// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Targeting/ablTargetingRaycast.h"

#include "ablAbilityDebug.h"
#include "ablSettings.h"
#include "Engine/World.h"

UAblTargetingRaycast::UAblTargetingRaycast(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Length(100.0f),
	m_OnlyWantBlockingObject(false)
{

}

UAblTargetingRaycast::~UAblTargetingRaycast()
{

}

void UAblTargetingRaycast::FindTargets(UAblAbilityContext& Context) const
{
	AActor* SourceActor = m_Location.GetSourceActor(Context);
	check(SourceActor);
	UWorld* World = SourceActor->GetWorld();
	FTransform QueryTransform;

	if (IsUsingAsync() && UAbleSettings::IsAsyncEnabled())
	{
		if (!Context.HasValidAsyncHandle())
		{
			m_Location.GetTransform(Context, QueryTransform);

			FVector Start = QueryTransform.GetLocation();
			FVector End = Start + QueryTransform.GetRotation().GetForwardVector() * m_Length;

			FTraceHandle AsyncHandle = World->AsyncLineTraceByChannel(m_OnlyWantBlockingObject ? EAsyncTraceType::Single : EAsyncTraceType::Multi, Start, End, GetCollisionChannel());
			Context.SetAsyncHandle(AsyncHandle);
		}
		else
		{
			FTraceDatum Datum;
			if (World->QueryTraceData(Context.GetAsyncHandle(), Datum))
			{
				ProcessResults(Context, Datum.OutHits);
				FTraceHandle Empty;
				Context.SetAsyncHandle(Empty);
			}

			return;
		}
	}
	else
	{
		m_Location.GetTransform(Context, QueryTransform);

		FVector Start = QueryTransform.GetLocation();
		FVector End = Start + QueryTransform.GetRotation().GetForwardVector() * m_Length;

		TArray<FHitResult> HitResults;
		if (m_OnlyWantBlockingObject)
		{
			FHitResult outResult;
			if (World->LineTraceSingleByChannel(outResult, Start, End, GetCollisionChannel()))
			{
				HitResults.Add(outResult);
			}
		}
		else
		{
			World->LineTraceMultiByChannel(HitResults, Start, End, GetCollisionChannel());
		}

		if (HitResults.Num() != 0)
		{
			ProcessResults(Context, HitResults);
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawRaycastQuery(World, QueryTransform, m_Length);
	}
#endif // UE_BUILD_SHIPPING
}

float UAblTargetingRaycast::CalculateRange() const
{
	if (m_CalculateAs2DRange)
	{
		return m_Location.GetOffset().Size2D() + m_Length;
	}

	return m_Location.GetOffset().Size() + m_Length;
}

void UAblTargetingRaycast::ProcessResults(UAblAbilityContext& Context, const TArray<struct FHitResult>& Results) const
{
	TArray<TWeakObjectPtr<AActor>>& TargetActors = Context.GetMutableTargetActors();

	for (const FHitResult& Result : Results)
	{
		if (AActor* ResultActor = Result.GetActor())
		{
			TargetActors.Add(ResultActor);
		}
	}

	// Run any extra filters.
	FilterTargets(Context);
}