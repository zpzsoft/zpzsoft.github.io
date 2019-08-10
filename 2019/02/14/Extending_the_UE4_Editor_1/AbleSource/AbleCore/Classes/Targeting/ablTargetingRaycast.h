// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Targeting/ablTargetingBase.h"
#include "UObject/ObjectMacros.h"

#include "ablTargetingRaycast.generated.h"

#define LOCTEXT_NAMESPACE "AbleCore"

UCLASS(EditInlineNew, meta = (DisplayName = "Raycast", ShortToolTip = "A ray cast based targeting query."))
class UAblTargetingRaycast : public UAblTargetingBase
{
	GENERATED_BODY()
public:
	UAblTargetingRaycast(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblTargetingRaycast();

	/* Find all the Targets with our Query. */
	virtual void FindTargets(UAblAbilityContext& Context) const override;
private:
	/* Calculate the range of the Query. */
	virtual float CalculateRange() const override;

	/* Helper Method to process the Targets of this query. */
	void ProcessResults(UAblAbilityContext& Context, const TArray<struct FHitResult>& Results) const;

	/* Length of the Ray */
	UPROPERTY(EditInstanceOnly, Category = "Raycast", meta = (DisplayName = "Length", ClampMin=0.01f))
	float m_Length;

	/* If True, this Target will *Only* return the object that caused the blocking hit. */
	UPROPERTY(EditInstanceOnly, Category = "Raycast", meta = (DisplayName = "Return Blocking Object", ClampMin = 0.01f))
	bool m_OnlyWantBlockingObject;
};

#undef LOCTEXT_NAMESPACE