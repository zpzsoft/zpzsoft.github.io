// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Targeting/ablTargetingCapsule.h"

#include "ablAbilityDebug.h"
#include "ablSettings.h"
#include "Engine/World.h"

UAblTargetingCapsule::UAblTargetingCapsule(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Height(100.0f),
	m_Radius(50.0f)
{

}

UAblTargetingCapsule::~UAblTargetingCapsule()
{

}

void UAblTargetingCapsule::FindTargets(UAblAbilityContext& Context) const
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

			FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(m_Radius, m_Height * 0.5f);

			FTraceHandle AsyncHandle = World->AsyncOverlapByChannel(QueryTransform.GetLocation(), QueryTransform.GetRotation(), GetCollisionChannel(), CapsuleShape);
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

		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(m_Radius, m_Height * 0.5f);

		TArray<FOverlapResult> Results;
		if (World->OverlapMultiByChannel(Results, QueryTransform.GetTranslation(), QueryTransform.GetRotation(), GetCollisionChannel(), CapsuleShape))
		{
			ProcessResults(Context, Results);
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawCapsuleQuery(World, QueryTransform, m_Radius, m_Height);
	}
#endif // UE_BUILD_SHIPPING
}

float UAblTargetingCapsule::CalculateRange() const
{
	FVector Capsule(m_Radius * 2.0f, m_Radius * 2.0f, m_Height);
	FVector RotatedCapsule;

	FQuat Rotation = FQuat(m_Location.GetRotation());

	RotatedCapsule = Rotation.GetForwardVector() + Capsule.X;
	RotatedCapsule += Rotation.GetRightVector() + Capsule.Y;
	RotatedCapsule += Rotation.GetUpVector() + Capsule.Z;

	if (m_CalculateAs2DRange)
	{
		return m_Location.GetOffset().Size2D() + RotatedCapsule.Size2D();
	}

	return m_Location.GetOffset().Size() + RotatedCapsule.Size();
}

void UAblTargetingCapsule::ProcessResults(UAblAbilityContext& Context, const TArray<struct FOverlapResult>& Results) const
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