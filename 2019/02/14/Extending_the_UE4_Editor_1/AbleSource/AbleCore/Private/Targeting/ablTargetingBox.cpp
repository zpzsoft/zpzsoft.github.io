// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Targeting/ablTargetingBox.h"

#include "ablAbilityDebug.h"
#include "ablSettings.h"
#include "Engine/World.h"

UAblTargetingBox::UAblTargetingBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_HalfExtents(50.0f, 50.0f, 50.0f)
{

}

UAblTargetingBox::~UAblTargetingBox()
{

}

void UAblTargetingBox::FindTargets(UAblAbilityContext& Context) const
{
	AActor* SourceActor = m_Location.GetSourceActor(Context);
	check(SourceActor);
	UWorld* World = SourceActor->GetWorld();
	FTransform QueryTransform;

	if (IsUsingAsync() && UAbleSettings::IsAsyncEnabled())
	{
		// Check if we have a valid Async handle already. 
		if (!Context.HasValidAsyncHandle())
		{
			FCollisionShape BoxShape = FCollisionShape::MakeBox(m_HalfExtents);

			m_Location.GetTransform(Context, QueryTransform);

			// Push our query out by our half extents so we aren't centered in the box.
			FQuat Rotation = QueryTransform.GetRotation();

			FVector HalfExtentsOffset = Rotation.GetForwardVector() * m_HalfExtents.X;

			QueryTransform *= FTransform(HalfExtentsOffset);

			FTraceHandle AsyncHandle = World->AsyncOverlapByChannel(QueryTransform.GetLocation(), QueryTransform.GetRotation(), GetCollisionChannel(), BoxShape);
			Context.SetAsyncHandle(AsyncHandle);
		}
		else // Poll and see if our query is done, if so - process it.
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
	else // Normal Sync Query
	{
		FCollisionShape BoxShape = FCollisionShape::MakeBox(m_HalfExtents);

		m_Location.GetTransform(Context, QueryTransform);

		// Push our query out by our half extents so we aren't centered in the box.
		FQuat Rotation = QueryTransform.GetRotation();

		FVector HalfExtentsOffset = Rotation.GetForwardVector() * m_HalfExtents.X;

		QueryTransform *= FTransform(HalfExtentsOffset);

		TArray<FOverlapResult> Results;
		if (World->OverlapMultiByChannel(Results, QueryTransform.GetLocation(), QueryTransform.GetRotation(), GetCollisionChannel(), BoxShape))
		{
			ProcessResults(Context, Results);
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		// Nope, go ahead and fire off our Async query.
		FVector AlignedBox = GetAlignedBox(Context, QueryTransform);

		FAblAbilityDebug::DrawBoxQuery(World, QueryTransform, AlignedBox);
	}
#endif // UE_BUILD_SHIPPING
}

float UAblTargetingBox::CalculateRange() const
{
	FVector RotatedBox;
	FQuat Rotation = FQuat(m_Location.GetRotation());

	RotatedBox = Rotation.GetForwardVector() + m_HalfExtents.X;
	RotatedBox += Rotation.GetRightVector() + m_HalfExtents.Y;
	RotatedBox += Rotation.GetUpVector() + m_HalfExtents.Z;

	if (m_CalculateAs2DRange)
	{
		return m_Location.GetOffset().Size2D() + RotatedBox.Size2D();
	}
	
	return m_Location.GetOffset().Size() + RotatedBox.Size();
}

FVector UAblTargetingBox::GetAlignedBox(const UAblAbilityContext& Context, FTransform& OutQueryTransform) const
{
	FVector AlignedBox;

	m_Location.GetTransform(Context, OutQueryTransform);

	const FQuat QueryRotation = OutQueryTransform.GetRotation();

	// Somewhere down the pipeline, the Box rotation is ignored, so go ahead and take care of it here.
	AlignedBox = QueryRotation.GetForwardVector() * m_HalfExtents.X;
	AlignedBox += QueryRotation.GetRightVector() * m_HalfExtents.Y;
	AlignedBox += QueryRotation.GetUpVector() * m_HalfExtents.Z;

	// Move out by our Half Extents.
	FVector HalfExtentsOffset = QueryRotation.GetForwardVector() * m_HalfExtents.X;
	OutQueryTransform *= FTransform(HalfExtentsOffset);

	return AlignedBox;
}

void UAblTargetingBox::ProcessResults(UAblAbilityContext& Context, const TArray<struct FOverlapResult>& Results) const
{
	TArray<TWeakObjectPtr<AActor>>& TargetActors = Context.GetMutableTargetActors();

	for (const FOverlapResult& Result : Results)
	{
		if (AActor* ResultActor = Result.GetActor())
		{
			TargetActors.Add(ResultActor);
		}
	}

	// Call our filters.
	FilterTargets(Context);
}
