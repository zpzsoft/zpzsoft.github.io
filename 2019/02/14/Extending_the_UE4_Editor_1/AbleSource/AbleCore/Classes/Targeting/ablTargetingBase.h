// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "ablAbilityContext.h"
#include "Engine/EngineTypes.h"
#include "UObject/ObjectMacros.h"

#include "ablTargetingBase.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTargeting"

class UPrimitiveComponent;
class UAblAbilityTargetingFilter;

/* A Simple struct for shared logic that takes in a various information and returns a Transform based on options selected by the user. */
USTRUCT()
struct ABLECORE_API FAblAbilityTargetTypeLocation
{
	GENERATED_USTRUCT_BODY()
public:
	FAblAbilityTargetTypeLocation();

	void GetTargetTransform(const UAblAbilityContext& Context, int32 TargetIndex, FTransform& OutTransform) const;
	void GetTransform(const UAblAbilityContext& Context, FTransform& OutTransform) const;
	AActor* GetSourceActor(const UAblAbilityContext& Context) const;
	EAblAbilityTargetType GetSourceTargetType() const { return m_Source.GetValue(); }
	
	FORCEINLINE const FVector& GetOffset() const { return m_Offset; }
	FORCEINLINE const FRotator& GetRotation() const { return m_Rotation; }
protected:
	/* The source to launch this targeting query from. All checks (distance, etc) will be in relation to this source. */
	UPROPERTY(EditInstanceOnly, Category = "Location", meta = (DisplayName = "Source"))
	TEnumAsByte<EAblAbilityTargetType> m_Source;

	/* An additional Offset to provide to our location. */
	UPROPERTY(EditInstanceOnly, Category = "Location", meta = (DisplayName = "Location Offset"))
	FVector m_Offset;

	/* Rotation to use for the location. */
	UPROPERTY(EditInstanceOnly, Category = "Location", meta = (DisplayName = "Rotation"))
	FRotator m_Rotation;

	/* Socket to use as the Base Transform or Location.*/
	UPROPERTY(EditInstanceOnly, Category = "Location", meta = (DisplayName = "Socket"))
	FName m_Socket;

	/* If true, the Socket rotation will be used as well as its location.*/
	UPROPERTY(EditInstanceOnly, Category = "Location", meta = (DisplayName = "Use Socket Rotation"))
	bool m_UseSocketRotation;
};

/* Base class for all our Targeting volumes/types. */
UCLASS(Abstract, EditInlineNew)
class ABLECORE_API UAblTargetingBase : public UObject
{
	GENERATED_BODY()
public:
	UAblTargetingBase(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblTargetingBase();

	/* Override in child classes, this method should find any targets according to the targeting volume/rules.*/
	virtual void FindTargets(UAblAbilityContext& Context) const { };
	
	/* Returns the Context Target Type to use as the source for any location-based logic. */
	FORCEINLINE EAblAbilityTargetType GetSource() const { return m_Location.GetSourceTargetType(); }

	/* Returns the Collision Channel to execute this query on. */
	FORCEINLINE ECollisionChannel GetCollisionChannel() const { return m_CollisionChannel.GetValue(); }

	/* Returns true if Targeting is using an Async query. */
	FORCEINLINE bool IsUsingAsync() const { return m_UseAsync; }

	/* Returns the range of this Targeting query.*/
	FORCEINLINE float GetRange() const { return m_Range; }

#if WITH_EDITOR
	// UObject Overrides.
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	/* Method for Child classes to override. This should calculate and return the range for the query. */
	virtual float CalculateRange() const { return 0.0f; }

	/* Runs all Targeting Filters. */
	void FilterTargets(UAblAbilityContext& Context) const;

	/* If true, the targeting range will be automatically calculated using shape, rotation, and offset information. This does not include socket offsets. */
	UPROPERTY(EditInstanceOnly, Category = "Targeting|Range", meta = (DisplayName = "Auto-calculate Range"))
	bool m_AutoCalculateRange;

	/* Range is primarily used by AI and other systems that have a target they wish to check against to avoid the full cost of the query. */
	UPROPERTY(EditInstanceOnly, Category = "Targeting|Range", meta = (DisplayName = "Range", EditCondition = "!m_AutoCalculateRange"))
	float m_Range;

	/* If true, when calculating the range, it will be calculated as a 2D distance (XY plane) rather than 3D. */
	UPROPERTY(EditInstanceOnly, Category = "Targeting|Range", meta = (DisplayName = "Calculate as 2D", EditCondition = "m_AutoCalculateRange"))
	bool m_CalculateAs2DRange;

	/* Where to begin our targeting query from. */
	UPROPERTY(EditInstanceOnly, Category="Targeting", meta=(DisplayName="Query Location"))
	FAblAbilityTargetTypeLocation m_Location;

	/* The collision channel to use when we perform the query. */
	UPROPERTY(EditInstanceOnly, Category = "Targeting", meta = (DisplayName = "Collision Channel"))
	TEnumAsByte<ECollisionChannel> m_CollisionChannel;

	/* Filters to run the initial results through. These are executed in order. */
	UPROPERTY(EditInstanceOnly, Instanced, Category = "Targeting", meta = (DisplayName = "Filters"))
	TArray<UAblAbilityTargetingFilter*> m_Filters;

	/* 
	*  If true, runs the targeting query on as an Async query rather than blocking the game thread. 
	*  This can lead to a performance increase but will cause a one frame delay before the query
	*  completes. If you don't need frame perfect execution - this is probably worth the small delay.
	*/
	UPROPERTY(EditInstanceOnly, Category = "Optimization", meta = (DisplayName = "Use Async"))
	bool m_UseAsync;
	
};

#undef LOCTEXT_NAMESPACE
