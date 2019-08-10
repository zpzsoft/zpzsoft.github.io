// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Engine/EngineTypes.h"
#include "Targeting/ablTargetingBase.h"
#include "UObject/ObjectMacros.h"

#include "ablCollisionFilters.generated.h"

struct FAblQueryResult;
class UAblAbilityContext;

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Base class for all Collision Filters. */
UCLASS(Abstract)
class ABLECORE_API UAblCollisionFilter : public UObject
{
	GENERATED_BODY()
public:
	UAblCollisionFilter(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilter();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Filter Self", ShortToolTip = "Filters out the Self Actor."))
class ABLECORE_API UAblCollisionFilterSelf : public UAblCollisionFilter
{
	GENERATED_BODY()
public:
	UAblCollisionFilterSelf(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilterSelf();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Filter Owner", ShortToolTip = "Filters out the Owner Actor."))
class ABLECORE_API UAblCollisionFilterOwner : public UAblCollisionFilter
{
	GENERATED_BODY()
public:
	UAblCollisionFilterOwner(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilterOwner();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Filter Instigator", ShortToolTip = "Filters out the Instigator Actor."))
class ABLECORE_API UAblCollisionFilterInstigator : public UAblCollisionFilter
{
	GENERATED_BODY()
public:
	UAblCollisionFilterInstigator(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilterInstigator();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Filter By Class", ShortToolTip = "Filters out any instances of the provided class, can also be negated to keep only instances of the provided class."))
class ABLECORE_API UAblCollisionFilterByClass : public UAblCollisionFilter
{
	GENERATED_BODY()
public:
	UAblCollisionFilterByClass(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilterByClass();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
protected:
	/* The Class to filter. */
	UPROPERTY(EditInstanceOnly, Category = "Filter", meta = (DisplayName = "Class"))
	const UClass* m_Class;

	/* If true, the filter will keep only items that are of the provided class rather than filter them out. */
	UPROPERTY(EditInstanceOnly, Category = "Filter", meta = (DisplayName = "Negate"))
	bool m_Negate;
};

UENUM()
enum EAblCollisionFilterSort
{
	AblFitlerSort_Ascending = 0 UMETA(DisplayName="Ascending"),
	AblFilterSort_Descending UMETA(DisplayName="Descending")
};

UCLASS(EditInlineNew, meta = (DisplayName = "Sort by Distance", ShortToolTip = "Sorts Targets from Nearest to Furthest (Ascending), or Furthest to Nearest (Descending)."))
class ABLECORE_API UAblCollisionFilterSortByDistance : public UAblCollisionFilter
{
	GENERATED_BODY()
public:
	UAblCollisionFilterSortByDistance(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilterSortByDistance();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
protected:
	/* Ascending will sort from closest to furthest, descending will be the opposite.*/
	UPROPERTY(EditInstanceOnly, Category = "Filter", meta = (DisplayName = "Sort Direction"))
	TEnumAsByte<EAblCollisionFilterSort> m_SortDirection;

	/* The Location to use as the source of our distance comparison. */
	UPROPERTY(EditInstanceOnly, Category = "Filter", meta = (DisplayName = "Source Location"))
	FAblAbilityTargetTypeLocation m_Location;

	/* If true, we will only use the 2D (XY Plane) distance rather than the 3D distance. */
	UPROPERTY(EditInstanceOnly, Category = "Filter", meta = (DisplayName = "Use 2D (XY) Distance)"))
	bool m_Use2DDistance;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Max Limit", ShortToolTip = "Keeps Target results limited to Max Entities."))
class ABLECORE_API UAblCollisionFilterMaxResults : public UAblCollisionFilter
{
	GENERATED_BODY()
public:
	UAblCollisionFilterMaxResults(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilterMaxResults();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
protected:
	/* The Maximum number of results you would like. Anything above this is removed.*/
	UPROPERTY(EditInstanceOnly, Category = "Filter", meta = (DisplayName = "Max Entities"))
	int32 m_MaxEntities;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Custom", ShortToolTip = "Calls the Ability's IsValidForActor Blueprint Event. If the event returns true, the actor is kept. If false, it is discarded."))
class ABLECORE_API UAblCollisionFilterCustom : public UAblCollisionFilter
{
	GENERATED_BODY()
public:
	UAblCollisionFilterCustom(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblCollisionFilterCustom();

	/* Perform our filter logic. */
	virtual void Filter(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<FAblQueryResult>& InOutArray) const;
protected:
	// Optional Name identifier for this event in case you are using IsValidForActor multiple times in the Ability.
	UPROPERTY(EditInstanceOnly, Category = "Filter", meta = (DisplayName = "Event Name"))
	FName m_EventName;

	// If true, the event is run across multiple actors on various cores. This can help speed things up if the potential actor list is large, or the BP logic is complex.
	UPROPERTY(EditInstanceOnly, Category = "Optimize", meta = (DisplayName = "Use Async"))
	bool m_UseAsync;
};

#undef LOCTEXT_NAMESPACE