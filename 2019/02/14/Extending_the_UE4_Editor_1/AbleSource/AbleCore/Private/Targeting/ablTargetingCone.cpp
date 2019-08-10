// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Targeting/ablTargetingCone.h"

#include "ablAbilityDebug.h"
#include "ablSettings.h"
#include "Engine/World.h"

UAblTargetingCone::UAblTargetingCone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_FOV(90.0f),
	m_Length(100.0f),
	m_Height(100.0f),
	m_2DQuery(true)
{

}

UAblTargetingCone::~UAblTargetingCone()
{

}

// For a Cone, we simply do a Sphere query pushed out to cover the entire cone, and then check if any entities are within a certain angle
// from our source. If our FOV is > 180, we take the inverse results (the cone represents a hole so we want nothing in it).

void UAblTargetingCone::FindTargets(UAblAbilityContext& Context) const
{
	AActor* SourceActor = m_Location.GetSourceActor(Context);
	check(SourceActor);
	UWorld* World = SourceActor->GetWorld();
	FTransform QueryTransform;

	if (IsUsingAsync() && UAbleSettings::IsAsyncEnabled())
	{
		if (!Context.HasValidAsyncHandle()) // Populate our Async query
		{
			const float Radius = (Is2DQuery() ? m_Length : FMath::Max(m_Height, m_Length)) * 0.5f;
			FCollisionShape SphereShape = FCollisionShape::MakeSphere(m_FOV < 180.0f ? Radius : Radius * 2.0f);

			m_Location.GetTransform(Context, QueryTransform);

			const FVector QueryForward = QueryTransform.GetRotation().GetForwardVector();

			// Center our position around our Radius.
			const FVector OffsetVector = QueryForward * Radius;
			const FVector QueryLocation = m_FOV > 180.0f ? QueryTransform.GetTranslation() : QueryTransform.GetTranslation() + OffsetVector;

			FTraceHandle AsyncHandle = World->AsyncOverlapByChannel(QueryLocation, FQuat::Identity, GetCollisionChannel(), SphereShape);
			Context.SetAsyncHandle(AsyncHandle);
			Context.SetAsyncQueryTransform(QueryTransform);
		}
		else // Poll for our results
		{
			FOverlapDatum Datum;
			if (World->QueryOverlapData(Context.GetAsyncHandle(), Datum))
			{
				ProcessResults(Context, Datum.OutOverlaps);
				FTraceHandle Empty;
				Context.SetAsyncHandle(Empty);
			}

			return;
		}
	}
	else // Sync Query
	{
		const float Radius = (Is2DQuery() ? m_Length : FMath::Max(m_Height, m_Length)) * 0.5f;
		FCollisionShape SphereShape = FCollisionShape::MakeSphere(m_FOV < 180.0f ? Radius : Radius * 2.0f);

		m_Location.GetTransform(Context, QueryTransform);

		const FVector QueryForward = QueryTransform.GetRotation().GetForwardVector();

		// Center our position around our Radius, unless we're great than 180, then we need center at our origin and increase our query size.
		const FVector OffsetVector = QueryForward * Radius;
		const FVector QueryLocation = m_FOV > 180.0f ? QueryTransform.GetTranslation() : QueryTransform.GetTranslation() + OffsetVector;

		TArray<FOverlapResult> Results;
		if (World->OverlapMultiByChannel(Results, QueryLocation, FQuat::Identity, GetCollisionChannel(), SphereShape))
		{
			Context.SetAsyncQueryTransform(QueryTransform); // Bit of a misnomer, but cache our transform here since ProcessResults will need it.
			ProcessResults(Context, Results);
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		if (Is2DQuery())
		{
			FAblAbilityDebug::Draw2DConeQuery(World, QueryTransform, m_FOV, m_Length);
		}
		else
		{
			FAblAbilityDebug::DrawConeQuery(World, QueryTransform, m_FOV, m_Length, m_Height);
		}
	}
#endif // UE_BUILD_SHIPPING

}

float UAblTargetingCone::CalculateRange() const
{
	if (Is2DQuery())
	{
		return m_Location.GetOffset().Size2D() + m_Length;
	}

	return m_Location.GetOffset().Size() + FMath::Max(m_Length, m_Height);
}

void UAblTargetingCone::ProcessResults(UAblAbilityContext& Context, const TArray<struct FOverlapResult>& Results) const
{
	// Cache off various values we use in our cone test.
	const bool GreaterThanOneEighty = m_FOV > 180.0f;
	const float QueryAngle = GreaterThanOneEighty ? 360.0f - m_FOV : m_FOV;
	const float HalfAngle = FMath::DegreesToRadians(QueryAngle * 0.5f);
	const float LengthSqr = m_Length * m_Length;
	const float HeightSqr = m_Height * m_Height;
	const float HeightAngle = Is2DQuery() ? 0.0f : FMath::Atan2(m_Height * 0.5f, m_Length);

	// Our output
	TArray<TWeakObjectPtr<AActor>>& TargetActors = Context.GetMutableTargetActors();

	// Grab the Transform we used.
	FTransform QueryTransform = Context.GetAsyncQueryTransform();

	FVector QueryForward = QueryTransform.GetRotation().GetForwardVector();

	// If we're great than 180, we take the angle of the "hole" and compare against that (which requires flipping our forward around).
	if (GreaterThanOneEighty)
	{
		// Flip it around.
		QueryForward = -QueryForward;
	}

	// Store all our Vectors to test against.
	const FVector QueryRight = QueryTransform.GetRotation().GetRightVector();
	const FVector QueryLocation = QueryTransform.GetTranslation();
	const FVector2D XYForward(QueryForward.X, QueryForward.Y); // Horiz Plane
	const FVector2D XZForward(QueryForward.X, QueryForward.Z); // Vert Plane

	// Various Parameters that will be written each check, declared here so we don't thrash the cache. 
	FTransform ResultTransform; // Our Overlap Result Transform.
	FVector ResultLocation; // Our Overlap Result Location (from our Transform).

	FVector ToTarget; // Vector from our Query Location to our Result. 
	FVector2D ToTargetXZ; // 2D Vector (XZ Plane) from our Query Location to our Result.
	FVector2D ToTargetXY; // 2D Vector (XY Plane) from our Query Location to our Result.

	bool ValidEntry = false; // Whether we've passed all checks or not.
	bool WithInAngle = false; // Whether we're within our Cone Angle 
	bool HalfSpace = false; // Half space check.

	float QueryToTargetDotProduct = 0.0f; // Dot product of our XYForward and ToTargetXY.
	float VerticalAngle = 0.0f;  // Angle, in radians, of our XZForward and ToTarget XZ.

	for (const FOverlapResult& Result : Results)
	{
		FAblQueryResult TempTarget(Result);

		TempTarget.GetTransform(ResultTransform);

		ResultLocation = ResultTransform.GetTranslation();

		ToTarget = ResultLocation - QueryLocation;
		ToTarget.Normalize();

		ToTargetXZ.Set(ToTarget.X, ToTarget.Z);
		ToTargetXY.Set(ToTarget.X, ToTarget.Y);

		ValidEntry = true;

		// If we're a 3D query, we base our initial sphere query on whichever is largest (height or length),
		// so do a quick distance check here, if those pass - go ahead and do our vertical angle check.
		if (!Is2DQuery())
		{
			if (FVector::DistSquared(ResultLocation, QueryLocation) > HeightSqr ||
				FVector::DistSquared(ResultLocation, QueryLocation) > LengthSqr)
			{
				ValidEntry = false;
			}
			else
			{
				VerticalAngle = FMath::Acos(FVector2D::DotProduct(XZForward, ToTargetXZ));

				ValidEntry = VerticalAngle < HeightAngle;
			}
		}

		// Move on to our Dot product checks
		if (ValidEntry)
		{
			// Check our Horizontal angle.
			QueryToTargetDotProduct = FVector2D::DotProduct(XYForward, ToTargetXY);

			WithInAngle = FMath::Acos(QueryToTargetDotProduct) < HalfAngle;

			HalfSpace = QueryToTargetDotProduct > 0.0f;

			ValidEntry = WithInAngle && HalfSpace;

			if (GreaterThanOneEighty) // If our FOV > 180 degrees, we want everything not in the angle check.
			{
				ValidEntry = !ValidEntry;
			}
		}

		// Save our success
		if (ValidEntry)
		{
			if (TempTarget.Actor.IsValid())
			{
				TargetActors.Add(TempTarget.Actor);
			}
		}
	}

	// Run our filters
	FilterTargets(Context);
}

