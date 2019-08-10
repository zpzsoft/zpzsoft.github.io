// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "ablAbilityContext.h"
#include "Engine/EngineTypes.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablCollisionSweepTypes.generated.h"

struct FAblQueryResult;

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Base class for all our Sweep Shapes. */
UCLASS(Abstract)
class ABLECORE_API UAblCollisionSweepShape : public UObject
{
	GENERATED_BODY()
public:
	UAblCollisionSweepShape(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionSweepShape();

	/* Perform the Synchronous Sweep. */
	virtual void DoSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform, TArray<FAblQueryResult>& OutResults) const { }
	
	/* Queue up the Async Sweep. */
	virtual FTraceHandle DoAsyncSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform) const { return FTraceHandle(); }
	
	/* Retrieve the Async results and process them. */
	virtual void GetAsyncResults(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTraceHandle& Handle, TArray<FAblQueryResult>& OutResults) const;

	/* Returns true if this shape is using Async. */
	FORCEINLINE bool IsAsync() const { return m_UseAsyncQuery; }

	/* Helper method to return the transform used in our Query. */
	void GetQueryTransform(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutTransform) const;

#if WITH_EDITOR
	virtual const FString DescribeShape() const { return FString(TEXT("None")); }
#endif

protected:
	/* The location of our Query. */
	UPROPERTY(EditInstanceOnly, Category="Sweep", meta=(DisplayName="Location"))
	FAblAbilityTargetTypeLocation m_SweepLocation;

	/* The collision channel to use when we perform the query. */
	UPROPERTY(EditInstanceOnly, Category = "Sweep", meta = (DisplayName = "Collision Channel"))
	TEnumAsByte<ECollisionChannel> m_CollisionChannel;

	/* If true, only return the blocking hit. Otherwise return all hits, including the blocking hit.*/
	UPROPERTY(EditInstanceOnly, Category = "Sweep", meta = (DisplayName = "Only Return Blocking Hit"))
	bool m_OnlyReturnBlockingHit;

	/* If true, the query is placed in the Async queue. This can help performance by spreading the query out by a frame or two. */
	UPROPERTY(EditInstanceOnly, Category = "Optimization", meta = (DisplayName = "Use Async Query"))
	bool m_UseAsyncQuery;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Box", ShortToolTip = "A box based sweep volume."))
class ABLECORE_API UAblCollisionSweepBox : public UAblCollisionSweepShape
{
	GENERATED_BODY()
public:
	UAblCollisionSweepBox(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionSweepBox();

	/* Perform the Synchronous Sweep. */
	virtual void DoSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform, TArray<FAblQueryResult>& OutResults) const override;
	
	/* Queue up the Async Sweep. */
	virtual FTraceHandle DoAsyncSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform) const override;

#if WITH_EDITOR
	virtual const FString DescribeShape() const;
#endif

protected:
	/* Half Extents of the Box */
	UPROPERTY(EditInstanceOnly, Category = "Box", meta = (DisplayName = "Half Extents"))
	FVector m_HalfExtents;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Sphere", ShortToolTip = "A sphere based sweep volume."))
class ABLECORE_API UAblCollisionSweepSphere : public UAblCollisionSweepShape
{
	GENERATED_BODY()
public:
	UAblCollisionSweepSphere(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionSweepSphere();

	/* Perform the Synchronous Sweep. */
	virtual void DoSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform, TArray<FAblQueryResult>& OutResults) const override;
	
	/* Queue up the Async Sweep. */
	virtual FTraceHandle DoAsyncSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform) const override;

#if WITH_EDITOR
	virtual const FString DescribeShape() const;
#endif

protected:
	/* Radius of the Sphere */
	UPROPERTY(EditInstanceOnly, Category = "Sphere", meta = (DisplayName = "Radius"))
	float m_Radius;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Capsule", ShortToolTip = "A capsule based sweep volume."))
class ABLECORE_API UAblCollisionSweepCapsule : public UAblCollisionSweepShape
{
	GENERATED_BODY()
public:
	UAblCollisionSweepCapsule(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionSweepCapsule();

	/* Perform the Synchronous Sweep. */
	virtual void DoSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform, TArray<FAblQueryResult>& OutResults) const override;
	
	/* Queue up the Async Sweep.*/
	virtual FTraceHandle DoAsyncSweep(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& SourceTransform) const override;

#if WITH_EDITOR
	virtual const FString DescribeShape() const;
#endif

protected:
	/* Radius of the Capsule */
	UPROPERTY(EditInstanceOnly, Category = "Capsule", meta = (DisplayName = "Radius"))
	float m_Radius;

	/* Height of the Capsule */
	UPROPERTY(EditInstanceOnly, Category = "Capsule", meta = (DisplayName = "Height"))
	float m_Height;
};

#undef LOCTEXT_NAMESPACE