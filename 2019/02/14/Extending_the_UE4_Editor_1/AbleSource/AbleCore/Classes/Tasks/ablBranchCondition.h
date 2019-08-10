// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "ablAbilityContext.h"
#include "Engine/EngineTypes.h"
#include "UObject/ObjectMacros.h"
#include "ablBranchCondition.generated.h"

class UAblAbility;
class UAblBranchTaskScratchPad;
class UPrimitiveComponent;

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Base class for Branch Conditions. */
UCLASS(Abstract, EditInlineNew)
class UAblBranchCondition : public UObject
{
	GENERATED_BODY()
public:
	UAblBranchCondition(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblBranchCondition();

	/* Returns if the result for this condition should be negated. */
	FORCEINLINE bool IsNegated() const { return m_Negate; }

	/* Checks the condition and returns the result. */
	virtual bool CheckCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context, UAblBranchTaskScratchPad& ScratchPad) const { return false; }
protected:
	// If true, it reverses the result of the logic check (true becomes false, false becomes true).
	UPROPERTY(EditInstanceOnly, Category = "Logic", meta = (DisplayName = "Negate"))
	bool m_Negate;
};

UCLASS(EditInlineNew, meta=(DisplayName="On Input", ShortToolTip="Branch if the player presses the given input."))
class UAblBranchConditionOnInput : public UAblBranchCondition
{
	GENERATED_BODY()
public:
	UAblBranchConditionOnInput(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblBranchConditionOnInput();

	/* Checks the condition and returns the result. */
	virtual bool CheckCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context, UAblBranchTaskScratchPad& ScratchPad) const override;
protected:
	// The input actions to look for. This expects the action keys to be bound to an action in the Input Settings.
	UPROPERTY(EditInstanceOnly, Category = "Input", meta = (DisplayName = "Input Actions"))
	TArray<FName> m_InputActions;

	// If true, the key must have been released before counting it as a press.
	UPROPERTY(EditInstanceOnly, Category = "Input", meta = (DisplayName = "Must Be Recently Pressed"))
	bool m_MustBeRecentlyPressed;

	// The time, in seconds, a button is allowed to be down to count as a "recent" press. Avoid 0.0 as floating point error makes that an unlikely value.
	UPROPERTY(EditInstanceOnly, Category = "Input", meta = (DisplayName = "Recently Pressed Time Limit", EditCondition="m_MustBeRecentlyPressed", ClampMin = 0.0f))
	float m_RecentlyPressedTimeLimit;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Always", ShortToolTip = "Always branch."))
class UAblBranchConditionAlways : public UAblBranchCondition
{
	GENERATED_BODY()
public:
	UAblBranchConditionAlways(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblBranchConditionAlways();

	/* Checks the condition and returns the result. */
	virtual bool CheckCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context, UAblBranchTaskScratchPad& ScratchPad) const override { return true; }
};

UCLASS(EditInlineNew, meta = (DisplayName = "Custom", ShortToolTip = "This will call into the owning Ability's CustomCanBranchTo Blueprint event to allow for any custom logic you wish to use."))
class UAblBranchConditionCustom : public UAblBranchCondition
{
	GENERATED_BODY()
public:
	UAblBranchConditionCustom(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblBranchConditionCustom();

	/* Checks the condition and returns the result. */
	virtual bool CheckCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context, UAblBranchTaskScratchPad& ScratchPad) const override;

};

#undef LOCTEXT_NAMESPACE