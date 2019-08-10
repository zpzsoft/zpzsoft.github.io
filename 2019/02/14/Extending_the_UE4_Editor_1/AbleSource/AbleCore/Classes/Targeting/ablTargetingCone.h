// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Targeting/ablTargetingBase.h"
#include "UObject/ObjectMacros.h"

#include "ablTargetingCone.generated.h"

#define LOCTEXT_NAMESPACE "AbleCore"

UCLASS(EditInlineNew, meta = (DisplayName = "Cone", ShortToolTip = "A cone based targeting volume."))
class UAblTargetingCone : public UAblTargetingBase
{
	GENERATED_BODY()
public:
	UAblTargetingCone(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblTargetingCone();

	/* Find any Targets.*/
	virtual void FindTargets(UAblAbilityContext& Context) const override;

	/* Returns whether this query is a 2D (XY Plane only) query. */
	FORCEINLINE bool Is2DQuery() const { return m_2DQuery; }
private:
	/* Calculates the range of this targeting.*/
	virtual float CalculateRange() const override;

	/* Helper method to process all potential results. */
	void ProcessResults(UAblAbilityContext& Context, const TArray<struct FOverlapResult>& Results) const;

	/* The Field of View (Angle/Azimuth) of the cone, in degrees. Supports Angles greater than 180 degrees. */
	UPROPERTY(EditInstanceOnly, Category = "Cone", meta = (DisplayName = "FOV", ClampMin=1.0f, ClampMax=360.0f))
	float m_FOV; // Azimuth

	/* Length of the Cone. */
	UPROPERTY(EditInstanceOnly, Category = "Cone", meta = (DisplayName = "Length", ClampMin=0.1f))
	float m_Length;

	/* Height of the Cone, ignored if using 2D/XY only. */
	UPROPERTY(EditInstanceOnly, Category = "Cone", meta = (DisplayName = "Height", ClampMin=0.1f))
	float m_Height;

	/* Whether to use a 2D (XY Plane) Cone, or 3D cone. */
	UPROPERTY(EditInstanceOnly, Category = "Targeting", meta = (DisplayName = "Use 2D Cone"))
	bool m_2DQuery;
};

#undef LOCTEXT_NAMESPACE