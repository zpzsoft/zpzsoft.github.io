// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Engine/EngineTypes.h"

#include "ablAbilityContext.h"

#include "Targeting/ablTargetingBase.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablCollisionQueryTypes.generated.h"

struct FAblQueryResult;

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Base class for all our Collision Query Shapes. */
UCLASS(Abstract)
class UAblCollisionShape : public UObject
{
	GENERATED_BODY()
public:
	UAblCollisionShape(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionShape();

	/* Perform the Synchronous Query.*/
	virtual void DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const { }
	
	/* Perform the Async Query.*/
	virtual FTraceHandle DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const { return FTraceHandle(); }

	/* Returns true, or false, if this Query is Async or not. */
	FORCEINLINE bool IsAsync() const { return m_UseAsyncQuery; }
	
	/* Returns the World this Query is occurring in.*/
	UWorld* GetQueryWorld(const TWeakObjectPtr<const UAblAbilityContext>& Context) const;

	/* Helper method to help process Async Query. */
	virtual void ProcessAsyncOverlaps(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& QueryTransform, const TArray<FOverlapResult>& Overlaps, TArray<FAblQueryResult>& OutResults) const;

#if WITH_EDITOR
	virtual const FString DescribeShape() const { return FString(TEXT("None")); }
#endif

protected:
	UPROPERTY(EditInstanceOnly, Category="Query", meta=(DisplayName="Query Location"))
	FAblAbilityTargetTypeLocation m_QueryLocation;

	/* The collision channel to use when we perform the query. */
	UPROPERTY(EditInstanceOnly, Category = "Query", meta = (DisplayName = "Collision Channel"))
	TEnumAsByte<ECollisionChannel> m_CollisionChannel;

	/* If true, the query is placed in the Async queue. This can help performance by spreading the query out by a frame or two. */
	UPROPERTY(EditInstanceOnly, Category = "Optimization", meta = (DisplayName = "Use Async Query"))
	bool m_UseAsyncQuery;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Box", ShortToolTip = "A box based query volume."))
class UAblCollisionShapeBox : public UAblCollisionShape
{
	GENERATED_BODY()
public:
	UAblCollisionShapeBox(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionShapeBox();

	/* Perform the Synchronous Query.*/
	virtual void DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const override;
	
	/* Perform the Async Query.*/
	virtual FTraceHandle DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const override;

#if WITH_EDITOR
	virtual const FString DescribeShape() const;
#endif

protected:
	/* Half Extents of the Box */
	UPROPERTY(EditInstanceOnly, Category = "Box", meta = (DisplayName = "Half Extents"))
	FVector m_HalfExtents;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Sphere", ShortToolTip = "A sphere based query volume."))
class UAblCollisionShapeSphere : public UAblCollisionShape
{
	GENERATED_BODY()
public:
	UAblCollisionShapeSphere(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionShapeSphere();

	/* Perform the Synchronous Query.*/
	virtual void DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const override;
	
	/* Perform the Async Query.*/
	virtual FTraceHandle DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const override;

#if WITH_EDITOR
	virtual const FString DescribeShape() const;
#endif

protected:
	/* Radius of the Sphere */
	UPROPERTY(EditInstanceOnly, Category = "Sphere", meta = (DisplayName = "Radius"))
	float m_Radius;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Capsule", ShortToolTip = "A capsule based query volume."))
class UAblCollisionShapeCapsule : public UAblCollisionShape
{
	GENERATED_BODY()
public:
	UAblCollisionShapeCapsule(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionShapeCapsule();

	/* Perform the Synchronous Query.*/
	virtual void DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const override;
	
	/* Perform the Async Query.*/
	virtual FTraceHandle DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const override;

#if WITH_EDITOR
	virtual const FString DescribeShape() const;
#endif

protected:
	/* Height of the Capsule */
	UPROPERTY(EditInstanceOnly, Category = "Capsule", meta = (DisplayName = "Height"))
	float m_Height;

	/* Radius of the Capsule */
	UPROPERTY(EditInstanceOnly, Category = "Capsule", meta = (DisplayName = "Radius"))
	float m_Radius;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Cone", ShortToolTip = "A cone based query volume, supports angles > 180 degrees."))
class UAblCollisionShapeCone : public UAblCollisionShape
{
	GENERATED_BODY()
public:
	UAblCollisionShapeCone(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionShapeCone();

	/* Do the Synchronous Query.*/
	virtual void DoQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& OutResults) const override;
	
	/* Do the Async Query.*/
	virtual FTraceHandle DoAsyncQuery(const TWeakObjectPtr<const UAblAbilityContext>& Context, FTransform& OutQueryTransform) const override;

	/* Helper method to help process our Async Query*/
	virtual void ProcessAsyncOverlaps(const TWeakObjectPtr<const UAblAbilityContext>& Context, const FTransform& QueryTransform, const TArray<FOverlapResult>& Overlaps, TArray<FAblQueryResult>& OutResults) const;

#if WITH_EDITOR
	virtual const FString DescribeShape() const;
#endif

protected:
	/* The Field of View (Angle/Azimuth) of the cone, in degrees. Supports Angles greater than 180 degrees. */
	UPROPERTY(EditInstanceOnly, Category = "Cone", meta = (DisplayName = "FOV", ClampMin = 1.0f, ClampMax = 360.0f))
	float m_FOV; // Azimuth

	/* Length of the Cone. */
	UPROPERTY(EditInstanceOnly, Category = "Cone", meta = (DisplayName = "Length", ClampMin = 0.1f))
	float m_Length;

	/* Height of the Cone */
	UPROPERTY(EditInstanceOnly, Category = "Cone", meta = (DisplayName = "Height", ClampMin = 0.1f, EditCondition="!m_Is2DQuery"))
	float m_Height;

	/* If true, the Height of the cone is ignored. */
	UPROPERTY(EditInstanceOnly, Category = "Cone", meta = (DisplayName = "Is 2D Query"))
	bool m_Is2DQuery;
};

#undef LOCTEXT_NAMESPACE