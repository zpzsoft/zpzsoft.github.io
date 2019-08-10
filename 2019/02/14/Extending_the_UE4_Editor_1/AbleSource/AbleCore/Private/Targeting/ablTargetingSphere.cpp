// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Targeting/ablTargetingSphere.h"

#include "ablAbilityDebug.h"
#include "ablSettings.h"
#include "Engine/World.h"

UAblTargetingSphere::UAblTargetingSphere(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Radius(100.0f)
{

}

UAblTargetingSphere::~UAblTargetingSphere()
{

}

void UAblTargetingSphere::FindTargets(UAblAbilityContext& Context) const
{
	AActor* SourceActor = m_Location.GetSourceActor(Context);
	check(SourceActor);
	UWorld* World = SourceActor->GetWorld();
	FTransform QueryTransform;

	if (IsUsingAsync() && UAbleSettings::IsAsyncEnabled())
	{
		if (!Context.HasValidAsyncHandle()) // If we don't have a handle, create our query.
		{
			m_Location.GetTransform(Context, QueryTransform);

			FCollisionShape SphereShape = FCollisionShape::MakeSphere(m_Radius);

			FTraceHandle AsyncHandle = World->AsyncOverlapByChannel(QueryTransform.GetLocation(), QueryTransform.GetRotation(), GetCollisionChannel(), SphereShape);
			Context.SetAsyncHandle(AsyncHandle);
		}
		else // Poll for completion.
		{
			FOverlapDatum Datum;
			if (World->QueryOverlapData(Context.GetAsyncHandle(), Datum))
			{
				ProcessResults(Context, Datum.OutOverlaps);
				FTraceHandle Empty;
				Context.SetAsyncHandle(Empty); // Reset our handle.
			}

			return;
		}
	}
	else // Sync Query
	{
		m_Location.GetTransform(Context, QueryTransform);

		FCollisionShape SphereShape = FCollisionShape::MakeSphere(m_Radius);

		TArray<FOverlapResult> Results;
		if (World->OverlapMultiByChannel(Results, QueryTransform.GetTranslation(), QueryTransform.GetRotation(), GetCollisionChannel(), SphereShape))
		{
			ProcessResults(Context, Results);
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawSphereQuery(World, QueryTransform, m_Radius);
	}
#endif // UE_BUILD_SHIPPING
}

float UAblTargetingSphere::CalculateRange() const
{
	const float OffsetSize = m_CalculateAs2DRange ? m_Location.GetOffset().Size2D() : m_Location.GetOffset().Size();

	return m_Radius + OffsetSize;
}

void UAblTargetingSphere::ProcessResults(UAblAbilityContext& Context, const TArray<struct FOverlapResult>& Results) const
{
	TArray<TWeakObjectPtr<AActor>>& TargetActors = Context.GetMutableTargetActors();

	for (const FOverlapResult& Result : Results)
	{
		if (AActor* ResultActor = Result.GetActor())
		{
			TargetActors.Add(ResultActor);
		}
	}

	// Run filters.
	FilterTargets(Context);
}