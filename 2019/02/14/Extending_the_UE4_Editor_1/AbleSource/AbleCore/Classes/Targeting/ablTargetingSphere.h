// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Targeting/ablTargetingBase.h"
#include "UObject/ObjectMacros.h"

#include "ablTargetingSphere.generated.h"

#define LOCTEXT_NAMESPACE "AbleCore"

UCLASS(EditInlineNew, meta = (DisplayName = "Sphere", ShortToolTip = "A sphere based targeting volume."))
class UAblTargetingSphere : public UAblTargetingBase
{
	GENERATED_BODY()
public:
	UAblTargetingSphere(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblTargetingSphere();

	/* Find all the Targets within our Query. */
	virtual void FindTargets(UAblAbilityContext& Context) const override;
private:
	/* Calculate the range of the Query. */
	virtual float CalculateRange() const override;

	/* Helper method to process the results of the Query. */
	void ProcessResults(UAblAbilityContext& Context, const TArray<struct FOverlapResult>& Results) const;

	/* Radius of the Sphere. */
	UPROPERTY(EditInstanceOnly, Category = "Sphere", meta = (DisplayName = "Radius", ClampMin=0.1f))
	float m_Radius;
};

#undef LOCTEXT_NAMESPACE