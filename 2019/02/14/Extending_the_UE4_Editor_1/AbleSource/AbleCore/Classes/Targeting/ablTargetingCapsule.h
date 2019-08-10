// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Targeting/ablTargetingBase.h"
#include "UObject/ObjectMacros.h"

#include "ablTargetingCapsule.generated.h"

#define LOCTEXT_NAMESPACE "AbleCore"

UCLASS(EditInlineNew, meta = (DisplayName = "Capsule", ShortToolTip = "A capsule based targeting volume."))
class UAblTargetingCapsule : public UAblTargetingBase
{
	GENERATED_BODY()
public:
	UAblTargetingCapsule(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblTargetingCapsule();

	/* Find all the Targets within our query.*/
	virtual void FindTargets(UAblAbilityContext& Context) const override;

private:
	/* Calculate the range of our Query.*/
	virtual float CalculateRange() const override;

	/* Process the results of our Query. */
	void ProcessResults(UAblAbilityContext& Context, const TArray<struct FOverlapResult>& Results) const;

	/* Height of the Capsule */
	UPROPERTY(EditInstanceOnly, Category = "Capsule", meta = (DisplayName = "Height"))
	float m_Height;

	/* Radius of the Capsule */
	UPROPERTY(EditInstanceOnly, Category = "Capsule", meta = (DisplayName = "Radius"))
	float m_Radius;
};

#undef LOCTEXT_NAMESPACE