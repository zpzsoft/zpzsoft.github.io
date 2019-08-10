// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Channeling/ablChannelingBase.h"
#include "Engine/EngineTypes.h"
#include "UObject/ObjectMacros.h"

#include "ablChannelingConditions.generated.h"
#define LOCTEXT_NAMESPACE "AblAbilityChanneling"

UCLASS(EditInlineNew, meta=(DisplayName="Input", ShortTooltip="Checks the provided Input Actions. If the key is pressed, it passes the condition - false otherwise"))
class ABLECORE_API UAblChannelingInputConditional : public UAblChannelingBase
{
	GENERATED_BODY()
public:
	UAblChannelingInputConditional(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblChannelingInputConditional();

protected:
	/* Override in your child class.*/
	virtual bool CheckConditional(UAblAbilityContext& Context) const override;

	// The input actions to look for. This expects the action keys to be bound to an action in the Input Settings.
	UPROPERTY(EditInstanceOnly, Category = "Conditional", meta = (DisplayName = "Input Actions"))
	TArray<FName> m_InputActions;

	mutable TArray<struct FKey> m_InputKeyCache;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Velocity", ShortTooltip = "Returns false if the Actor's velocity is above the provided threshold."))
class ABLECORE_API UAblChannelingVelocityConditional : public UAblChannelingBase
{
	GENERATED_BODY()
public:
	UAblChannelingVelocityConditional(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblChannelingVelocityConditional();
protected:
	/* Override in your child class.*/
	virtual bool CheckConditional(UAblAbilityContext& Context) const override;

	// The speed (in cm/s) to check the actor velocity against.
	UPROPERTY(EditInstanceOnly, Category = "Conditional", meta = (DisplayName = "Velocity Threshold"))
	float m_VelocityThreshold;

	// If true, we'll only check the 2D (XY) speed of the actor.
	UPROPERTY(EditInstanceOnly, Category = "Conditional", meta = (DisplayName = "Use XY Speed Only"))
	bool m_UseXYSpeed;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Custom", ShortTooltip = "Calls the Ability's CheckCustomChannelConditional method and returns the result."))
class ABLECORE_API UAblChannelingCustomConditional : public UAblChannelingBase
{
	GENERATED_BODY()
public:
	UAblChannelingCustomConditional(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblChannelingCustomConditional();
protected:
	/* Override in your child class.*/
	virtual bool CheckConditional(UAblAbilityContext& Context) const override;

	// Optional Name to give this condition, it will be passed to the Ability when calling the CheckCustomChannelConditional method.
	UPROPERTY(EditInstanceOnly, Category = "Conditional", meta = (DisplayName = "Event Name"))
	FName m_EventName;
};


#undef LOCTEXT_NAMESPACE
