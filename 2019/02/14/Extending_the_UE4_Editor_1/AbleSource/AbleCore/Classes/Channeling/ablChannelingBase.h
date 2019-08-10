// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "ablAbilityContext.h"
#include "UObject/ObjectMacros.h"
#include "ablChannelingBase.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityChanneling"

UCLASS(Abstract, EditInlineNew)
class ABLECORE_API UAblChannelingBase : public UObject
{
	GENERATED_BODY()
public:
	UAblChannelingBase(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblChannelingBase();

	bool GetConditionResult(UAblAbilityContext& Context) const;
protected:
	/* Override in your child class.*/
	virtual bool CheckConditional(UAblAbilityContext& Context) const { return true; }

	/* Negates the result of the condition (so false becomes true and vice versa). */
	UPROPERTY(EditInstanceOnly, Category = "Conditional", meta=(DisplayName="Negate"))
	bool m_Negate;

};

#undef LOCTEXT_NAMESPACE
