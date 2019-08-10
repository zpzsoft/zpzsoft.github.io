// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablCollisionSweepTypes.h"

#include "ablAbilityDebug.h"
#include "Components/SkeletalMeshComponent.h"

UAblCollisionSweepShape::UAblCollisionSweepShape(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_CollisionChannel(ECC_WorldDynamic),
	m_UseAsyncQuery(false)
{

}

UAblCollisionSweepShape::~UAblCollisionSweepShape()
{

}

void UAblCollisionSweepShape::GetAsyncResults(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTraceHandle& Handle, TArray<FAblQueryResult>& OutResults) const
{
	check(Context.IsValid());
	const UAblAbilityContext& ConstContext = *Context.Get();
	AActor* SourceActor = m_SweepLocation.GetSourceActor(ConstContext);
	check(SourceActor);

	UWorld* World = SourceActor->GetWorld();
	check(World);

	FTraceDatum Datum;
	if (World->QueryTraceData(Handle, Datum))
	{
		for (const FHitResult& HitResult : Datum.OutHits)
		{
			OutResults.Add(FAblQueryResult(HitResult));
		}
	}
}

void UAblCollisionSweepShape::GetQueryTransform(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutTransform) const
{
	check(Context.IsValid());
	m_SweepLocation.GetTransform(*Context.Get(), OutTransform);
}

UAblCollisionSweepBox::UAblCollisionSweepBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_HalfExtents(50.0f, 50.0f, 50.0f)
{

}

UAblCollisionSweepBox::~UAblCollisionSweepBox()
{

}

void UAblCollisionSweepBox::DoSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform, TArray<FAblQueryResult>& OutResults) const
{
	check(Context.IsValid());
	const UAblAbilityContext& ConstContext = *Context.Get();
	AActor* SourceActor = m_SweepLocation.GetSourceActor(ConstContext);
	check(SourceActor);

	FTransform StartTransform, EndTransform;
	m_SweepLocation.GetTransform(ConstContext, EndTransform);

	FQuat SourceRotation = SourceTransform.GetRotation();
	FVector StartOffset = SourceRotation.GetForwardVector() * m_HalfExtents.X;

	StartTransform = SourceTransform * FTransform(StartOffset);

	FQuat EndRotation = EndTransform.GetRotation();
	FVector EndOffset = EndRotation.GetForwardVector() * m_HalfExtents.X;

	EndTransform *= FTransform(EndOffset);

	UWorld* World = SourceActor->GetWorld();
	check(World);

	FCollisionShape Shape = FCollisionShape::MakeBox(m_HalfExtents);
	if (m_OnlyReturnBlockingHit)
	{
		FHitResult SweepResult;
		if (World->SweepSingleByChannel(SweepResult, StartTransform.GetLocation(), EndTransform.GetLocation(), SourceTransform.GetRotation(), m_CollisionChannel, Shape))
		{
			OutResults.Add(FAblQueryResult(SweepResult));
		}
	}
	else
	{
		TArray<FHitResult> SweepResults;
		if (World->SweepMultiByChannel(SweepResults, StartTransform.GetLocation(), EndTransform.GetLocation(), SourceTransform.GetRotation(), m_CollisionChannel, Shape))
		{
			for (const FHitResult& SweepResult : SweepResults)
			{
				OutResults.Add(FAblQueryResult(SweepResult));
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FVector AlignedBox = StartTransform.GetRotation().GetForwardVector() * m_HalfExtents.X;
		AlignedBox += StartTransform.GetRotation().GetRightVector() * m_HalfExtents.Y;
		AlignedBox += StartTransform.GetRotation().GetUpVector() * m_HalfExtents.Z;

		FAblAbilityDebug::DrawBoxSweep(World, StartTransform, EndTransform, AlignedBox);
	}
#endif
}

FTraceHandle UAblCollisionSweepBox::DoAsyncSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform) const
{
	check(Context.IsValid());
	const UAblAbilityContext& ConstContext = *Context.Get();
	AActor* SourceActor = m_SweepLocation.GetSourceActor(ConstContext);
	check(SourceActor);

	FTransform StartTransform, EndTransform;
	m_SweepLocation.GetTransform(ConstContext, EndTransform);

	FQuat SourceRotation = SourceTransform.GetRotation();
	FVector StartOffset = SourceRotation.GetForwardVector() * m_HalfExtents.X;

	StartTransform = SourceTransform * FTransform(StartOffset);

	FQuat EndRotation = EndTransform.GetRotation();
	FVector EndOffset = EndRotation.GetForwardVector() * m_HalfExtents.X;

	EndTransform *= FTransform(EndOffset);

	UWorld* World = SourceActor->GetWorld();
	check(World);

	FCollisionShape Shape = FCollisionShape::MakeBox(m_HalfExtents);

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FVector AlignedBox = StartTransform.GetRotation().GetForwardVector() * m_HalfExtents.X;
		AlignedBox += StartTransform.GetRotation().GetRightVector() * m_HalfExtents.Y;
		AlignedBox += StartTransform.GetRotation().GetUpVector() * m_HalfExtents.Z;

		FAblAbilityDebug::DrawBoxSweep(World, StartTransform, EndTransform, AlignedBox);
	}
#endif

	return World->AsyncSweepByChannel(m_OnlyReturnBlockingHit ? EAsyncTraceType::Single : EAsyncTraceType::Multi, StartTransform.GetLocation(), EndTransform.GetLocation(), m_CollisionChannel, Shape);
}

#if WITH_EDITOR

const FString UAblCollisionSweepBox::DescribeShape() const
{
	return FString::Printf(TEXT("Box %.1fm x %.1fm x%.1fm"), m_HalfExtents.X * 2.0f * 0.01f, m_HalfExtents.Y * 2.0f * 0.01f, m_HalfExtents.Z * 2.0f * 0.01f);
}

#endif

UAblCollisionSweepSphere::UAblCollisionSweepSphere(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer),
	m_Radius(50.0f)
{

}

UAblCollisionSweepSphere::~UAblCollisionSweepSphere()
{

}

void UAblCollisionSweepSphere::DoSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform, TArray<FAblQueryResult>& OutResults) const
{
	check(Context.IsValid());
	const UAblAbilityContext& ConstContext = *Context.Get();
	AActor* SourceActor = m_SweepLocation.GetSourceActor(ConstContext);
	check(SourceActor);

	FTransform EndTransform;
	m_SweepLocation.GetTransform(ConstContext, EndTransform);

	UWorld* World = SourceActor->GetWorld();
	check(World);

	FCollisionShape Shape = FCollisionShape::MakeSphere(m_Radius);
	if (m_OnlyReturnBlockingHit)
	{
		FHitResult SweepResult;
		if (World->SweepSingleByChannel(SweepResult, SourceTransform.GetLocation(), EndTransform.GetLocation(), FQuat::Identity, m_CollisionChannel, Shape))
		{
			OutResults.Add(FAblQueryResult(SweepResult));
		}
	}
	else
	{
		TArray<FHitResult> SweepResults;
		if (World->SweepMultiByChannel(SweepResults, SourceTransform.GetLocation(), EndTransform.GetLocation(), FQuat::Identity, m_CollisionChannel, Shape))
		{
			for (const FHitResult& SweepResult : SweepResults)
			{
				OutResults.Add(FAblQueryResult(SweepResult));
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawSphereSweep(World, SourceTransform, EndTransform, m_Radius);
	}
#endif

}

FTraceHandle UAblCollisionSweepSphere::DoAsyncSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform) const
{
	check(Context.IsValid());
	const UAblAbilityContext& ConstContext = *Context.Get();
	AActor* SourceActor = m_SweepLocation.GetSourceActor(ConstContext);
	check(SourceActor);

	FTransform EndTransform;
	m_SweepLocation.GetTransform(ConstContext, EndTransform);

	UWorld* World = SourceActor->GetWorld();
	check(World);

	FCollisionShape Shape = FCollisionShape::MakeSphere(m_Radius);

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawSphereSweep(World, SourceTransform, EndTransform, m_Radius);
	}
#endif

	return World->AsyncSweepByChannel(m_OnlyReturnBlockingHit ? EAsyncTraceType::Single : EAsyncTraceType::Multi, SourceTransform.GetLocation(), EndTransform.GetLocation(), m_CollisionChannel, Shape);
}

#if WITH_EDITOR

const FString UAblCollisionSweepSphere::DescribeShape() const
{
	return FString::Printf(TEXT("Sphere %.1fm"), m_Radius * 2.0f * 0.01f);
}

#endif

UAblCollisionSweepCapsule::UAblCollisionSweepCapsule(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Radius(25.0f),
	m_Height(100.0f)
{

}

UAblCollisionSweepCapsule::~UAblCollisionSweepCapsule()
{

}

void UAblCollisionSweepCapsule::DoSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform, TArray<FAblQueryResult>& OutResults) const
{
	check(Context.IsValid());
	const UAblAbilityContext& ConstContext = *Context.Get();
	AActor* SourceActor = m_SweepLocation.GetSourceActor(ConstContext);
	check(SourceActor);

	FTransform EndTransform;
	m_SweepLocation.GetTransform(ConstContext, EndTransform);

	UWorld* World = SourceActor->GetWorld();
	check(World);

	FCollisionShape Shape = FCollisionShape::MakeCapsule(m_Radius, m_Height * 0.5f);
	if (m_OnlyReturnBlockingHit)
	{
		FHitResult SweepResult;
		if (World->SweepSingleByChannel(SweepResult, SourceTransform.GetLocation(), EndTransform.GetLocation(), SourceTransform.GetRotation(), m_CollisionChannel, Shape))
		{
			OutResults.Add(FAblQueryResult(SweepResult));
		}
	}
	else
	{
		TArray<FHitResult> SweepResults;
		if (World->SweepMultiByChannel(SweepResults, SourceTransform.GetLocation(), EndTransform.GetLocation(), SourceTransform.GetRotation(), m_CollisionChannel, Shape))
		{
			for (const FHitResult& SweepResult : SweepResults)
			{
				OutResults.Add(FAblQueryResult(SweepResult));
			}
		}
	}

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawCapsuleSweep(World, SourceTransform, EndTransform, m_Radius, m_Height);
	}
#endif

}

FTraceHandle UAblCollisionSweepCapsule::DoAsyncSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform) const
{
	check(Context.IsValid());
	const UAblAbilityContext& ConstContext = *Context.Get();
	AActor* SourceActor = m_SweepLocation.GetSourceActor(ConstContext);
	check(SourceActor);

	FTransform EndTransform;
	m_SweepLocation.GetTransform(ConstContext, EndTransform);

	UWorld* World = SourceActor->GetWorld();
	check(World);

	FCollisionShape Shape = FCollisionShape::MakeCapsule(m_Radius, m_Height * 0.5f);

#if !UE_BUILD_SHIPPING
	if (FAblAbilityDebug::ShouldDrawQueries())
	{
		FAblAbilityDebug::DrawCapsuleSweep(World, SourceTransform, EndTransform, m_Radius, m_Height);
	}
#endif

	return World->AsyncSweepByChannel(m_OnlyReturnBlockingHit ? EAsyncTraceType::Single : EAsyncTraceType::Multi, SourceTransform.GetLocation(), EndTransform.GetLocation(), m_CollisionChannel, Shape);
}

#if WITH_EDITOR

const FString UAblCollisionSweepCapsule::DescribeShape() const
{
	return FString::Printf(TEXT("Capsule %.1fm x %.1fm"), m_Height * 0.01f, m_Radius * 2.0f * 0.01f);
}

#endif