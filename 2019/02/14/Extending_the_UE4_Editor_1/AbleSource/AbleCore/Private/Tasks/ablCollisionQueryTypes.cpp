// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablCollisionQueryTypes.h"

#include "ablAbilityDebug.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Tasks/ablCollisionQueryTask.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblCollisionShape::UAblCollisionShape(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_CollisionChannel(ECC_WorldDynamic),
	m_UseAsyncQuery(false)
{

}

UAblCollisionShape::~UAblCollisionShape()
{

}

UWorld* UAblCollisionShape::GetQueryWorld(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	if (AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get()))
	{
		return SourceActor->GetWorld();
	}

	return GEngine->GetWorld();
}

void UAblCollisionShape::ProcessAsyncOverlaps(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& QueryTransform, const TArray<FOverlapResult>& Overlaps, TArray<FAblQueryResult>& OutResults) const
{
	for (const FOverlapResult& Result : Overlaps)
	{
		OutResults.Add(FAblQueryResult(Result));
	}
}

UAblCollisionShapeBox::UAblCollisionShapeBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_HalfExtents(100.0f, 100.0f, 100.0f)
{

}

UAblCollisionShapeBox::~UAblCollisionShapeBox()
{

}

void UAblCollisionShapeBox::DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	FTransform QueryTransform;
	
	m_QueryLocation.GetTransform(*Context.Get(), QueryTransform);

	// Push our query out by our half extents so we aren't centered in the box.
	FQuat Rotation = QueryTransform.GetRotation();

	FVector HalfExtentsOffset = Rotation.GetForwardVector() * m_HalfExtents.X;

	QueryTransform *= FTransform(HalfExtentsOffset);

	FCollisionShape Box = FCollisionShape::MakeBox(m_HalfExtents);

	TArray<FOverlapResult> OverlapResults;
	if (World->OverlapMultiByChannel(OverlapResults, QueryTransform.GetLocation(), QueryTransform.GetRotation(), m_CollisionChannel.GetValue(), Box))
	{
		for (FOverlapResult& Result : OverlapResults)
		{
			OutResults.Add(FAblQueryResult(Result));
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FVector AlignedBox = QueryTransform.GetRotation().GetForwardVector() * m_HalfExtents.X;
		AlignedBox += QueryTransform.GetRotation().GetRightVector() * m_HalfExtents.Y;
		AlignedBox += QueryTransform.GetRotation().GetUpVector() * m_HalfExtents.Z;

		FAblAbilityDebug::DrawBoxQuery(World, QueryTransform, AlignedBox);
	}
#endif

}

FTraceHandle UAblCollisionShapeBox::DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	m_QueryLocation.GetTransform(*Context.Get(), OutQueryTransform);

	// Push our query out by our half extents so we aren't centered in the box.
	FQuat Rotation = OutQueryTransform.GetRotation();

	FVector HalfExtentsOffset = Rotation.GetForwardVector() * m_HalfExtents.X;

	OutQueryTransform *= FTransform(HalfExtentsOffset);

	FCollisionShape Box = FCollisionShape::MakeBox(m_HalfExtents);

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FVector AlignedBox = OutQueryTransform.GetRotation().GetForwardVector() * m_HalfExtents.X;
		AlignedBox += OutQueryTransform.GetRotation().GetRightVector() * m_HalfExtents.Y;
		AlignedBox += OutQueryTransform.GetRotation().GetUpVector() * m_HalfExtents.Z;

		FAblAbilityDebug::DrawBoxQuery(World, OutQueryTransform, AlignedBox);
	}
#endif

	return World->AsyncOverlapByChannel(OutQueryTransform.GetLocation(), FQuat::Identity, m_CollisionChannel.GetValue(), Box);
}

#if WITH_EDITOR

const FString UAblCollisionShapeBox::DescribeShape() const
{
	return FString::Printf(TEXT("Box %.1fm x %.1fm x%.1fm"), m_HalfExtents.X * 2.0f * 0.01f, m_HalfExtents.Y * 2.0f * 0.01f, m_HalfExtents.Z * 2.0f * 0.01f);
}

#endif

UAblCollisionShapeSphere::UAblCollisionShapeSphere(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Radius(50.0f)
{

}

UAblCollisionShapeSphere::~UAblCollisionShapeSphere()
{

}

void UAblCollisionShapeSphere::DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	FTransform QueryTransform;
	m_QueryLocation.GetTransform(*Context.Get(), QueryTransform);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(m_Radius);

	TArray<FOverlapResult> OverlapResults;
	if (World->OverlapMultiByChannel(OverlapResults, QueryTransform.GetLocation(), QueryTransform.GetRotation(), m_CollisionChannel.GetValue(), Sphere))
	{
		for (FOverlapResult& Result : OverlapResults)
		{
			OutResults.Add(FAblQueryResult(Result));
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawSphereQuery(World, QueryTransform, m_Radius);
	}
#endif
}

FTraceHandle UAblCollisionShapeSphere::DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	m_QueryLocation.GetTransform(*Context.Get(), OutQueryTransform);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(m_Radius);

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawSphereQuery(World, OutQueryTransform, m_Radius);
	}
#endif

	return World->AsyncOverlapByChannel(OutQueryTransform.GetLocation(), OutQueryTransform.GetRotation(), m_CollisionChannel.GetValue(), Sphere);
}

#if WITH_EDITOR

const FString UAblCollisionShapeSphere::DescribeShape() const
{
	return FString::Printf(TEXT("Sphere %.1fm"), m_Radius * 2.0f * 0.01f);
}

#endif

UAblCollisionShapeCapsule::UAblCollisionShapeCapsule(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Height(100.0f),
	m_Radius(50.0f)
{

}

UAblCollisionShapeCapsule::~UAblCollisionShapeCapsule()
{

}

void UAblCollisionShapeCapsule::DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	FTransform QueryTransform;
	m_QueryLocation.GetTransform(*Context.Get(), QueryTransform);

	FCollisionShape Capsule = FCollisionShape::MakeCapsule(m_Radius, m_Height * 0.5f);

	TArray<FOverlapResult> OverlapResults;
	if (World->OverlapMultiByChannel(OverlapResults, QueryTransform.GetLocation(), QueryTransform.GetRotation(), m_CollisionChannel.GetValue(), Capsule))
	{
		for (FOverlapResult& Result : OverlapResults)
		{
			OutResults.Add(FAblQueryResult(Result));
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawCapsuleQuery(World, QueryTransform, m_Radius, m_Height);
	}
#endif
}

FTraceHandle UAblCollisionShapeCapsule::DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	m_QueryLocation.GetTransform(*Context.Get(), OutQueryTransform);

	FCollisionShape Capsule = FCollisionShape::MakeCapsule(m_Radius, m_Height * 0.5f);

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawCapsuleQuery(World, OutQueryTransform, m_Radius, m_Height);
	}
#endif

	return World->AsyncOverlapByChannel(OutQueryTransform.GetLocation(), OutQueryTransform.GetRotation(), m_CollisionChannel.GetValue(), Capsule);
}

#if WITH_EDITOR

const FString UAblCollisionShapeCapsule::DescribeShape() const
{
	return FString::Printf(TEXT("Capsule %.1fm x %.1fm"), m_Height * 0.01f, m_Radius * 2.0f * 0.01f);
}

#endif

UAblCollisionShapeCone::UAblCollisionShapeCone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_FOV(90.0f),
	m_Length(100.0f),
	m_Height(50.0f),
	m_Is2DQuery(false)
{

}

UAblCollisionShapeCone::~UAblCollisionShapeCone()
{

}

void UAblCollisionShapeCone::DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	const float Radius = (m_Is2DQuery ? m_Length : FMath::Max(m_Height, m_Length)) * 0.5f;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(m_FOV < 180.0f ? Radius : Radius * 2.0f);
	
	FTransform QueryTransform;
	m_QueryLocation.GetTransform(*Context.Get(), QueryTransform);

	const FVector QueryForward = QueryTransform.GetRotation().GetForwardVector();

	// Center our position around our Radius, unless we're great than 180, then we need center at our origin and increase our query size.
	const FVector OffsetVector = QueryForward * Radius;
	const FVector QueryLocation = m_FOV > 180.0f ? QueryTransform.GetTranslation() : QueryTransform.GetTranslation() + OffsetVector;

	TArray<FOverlapResult> OverlapResults;
	if (World->OverlapMultiByChannel(OverlapResults, QueryLocation, QueryTransform.GetRotation(), m_CollisionChannel.GetValue(), SphereShape))
	{
		// Do our actual cone logic, just use the Async method to keep the logic in one place.
		ProcessAsyncOverlaps(Context, QueryTransform, OverlapResults, OutResults);
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		if (m_Is2DQuery)
		{
			FAblAbilityDebug::Draw2DConeQuery(World, QueryTransform, m_FOV, m_Length);
		}
		else
		{
			FAblAbilityDebug::DrawConeQuery(World, QueryTransform, m_FOV, m_Length, m_Height);
		}
	}
#endif
}

FTraceHandle UAblCollisionShapeCone::DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const
{
	AActor* SourceActor = m_QueryLocation.GetSourceActor(*Context.Get());
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();

	const float Radius = (m_Is2DQuery ? m_Length : FMath::Max(m_Height, m_Length)) * 0.5f;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(m_FOV < 180.0f ? Radius : Radius * 2.0f);

	m_QueryLocation.GetTransform(*Context.Get(), OutQueryTransform);

	const FVector QueryForward = OutQueryTransform.GetRotation().GetForwardVector();

	// Center our position around our Radius, unless we're great than 180, then we need center at our origin and increase our query size.
	const FVector OffsetVector = QueryForward * Radius;
	const FVector QueryLocation = m_FOV > 180.0f ? OutQueryTransform.GetTranslation() : OutQueryTransform.GetTranslation() + OffsetVector;

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		if (m_Is2DQuery)
		{
			FAblAbilityDebug::Draw2DConeQuery(World, OutQueryTransform, m_FOV, m_Length);
		}
		else
		{
			FAblAbilityDebug::DrawConeQuery(World, OutQueryTransform, m_FOV, m_Length, m_Height);
		}
	}
#endif

	return World->AsyncOverlapByChannel(QueryLocation, OutQueryTransform.GetRotation(), m_CollisionChannel.GetValue(), SphereShape);
}

void UAblCollisionShapeCone::ProcessAsyncOverlaps(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& QueryTransform, const TArray<FOverlapResult>& Overlaps, TArray<FAblQueryResult>& OutResults) const
{
	// Cache off various values we use in our cone test.
	const bool GreaterThanOneEighty = m_FOV > 180.0f;
	const float QueryAngle = GreaterThanOneEighty ? 360.0f - m_FOV : m_FOV;
	const float HalfAngle = FMath::DegreesToRadians(QueryAngle * 0.5f);
	const float LengthSqr = m_Length * m_Length;
	const float HeightSqr = m_Height * m_Height;
	const float HeightAngle = m_Is2DQuery ? 0.0f : FMath::Atan2(m_Height * 0.5f, m_Length);

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

	for (const FOverlapResult& Result : Overlaps)
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
		if (!m_Is2DQuery)
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
			OutResults.Add(TempTarget);
		}
	}

}

#if WITH_EDITOR

const FString UAblCollisionShapeCone::DescribeShape() const
{
	if (m_Is2DQuery)
	{
		return FString::Printf(TEXT("Cone %.1fdeg %.1fm"), m_FOV, m_Length * 0.01f);
	}
	
	return FString::Printf(TEXT("Cone %.1fdeg %.1fm x %.1fm"), m_FOV, m_Length * 0.01f, m_Height * 0.01f);
}

#endif

#undef LOCTEXT_NAMESPACE