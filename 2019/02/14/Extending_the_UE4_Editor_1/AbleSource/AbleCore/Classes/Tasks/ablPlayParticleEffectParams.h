// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Targeting/ablTargetingBase.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablPlayParticleEffectParams.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Base class for all our Particle Effect Parameters. */
UCLASS(Abstract)
class UAblParticleEffectParam : public UObject
{
	GENERATED_BODY()
public:
	UAblParticleEffectParam(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer), m_ParameterName(NAME_None) { }
	virtual ~UAblParticleEffectParam() { }

	FORCEINLINE const FName GetParameterName() const { return m_ParameterName; }

private:
	UPROPERTY(EditInstanceOnly, Category = "Parameter", meta=(DisplayName="Property Name"))
	FName m_ParameterName;
};

UCLASS(EditInlineNew, meta=(DisplayName="Context Actor", ShortToolTip="Reference a Context Actor"))
class UAblParticleEffectParamContextActor : public UAblParticleEffectParam
{
	GENERATED_BODY()
public:
	UAblParticleEffectParamContextActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), m_ContextActor(EAblAbilityTargetType::Self) { }
	virtual ~UAblParticleEffectParamContextActor() { }

	FORCEINLINE EAblAbilityTargetType GetContextActorType() const { return m_ContextActor.GetValue(); }
private:
	UPROPERTY(EditInstanceOnly, Category="Parameter", meta=(DisplayName="Context Actor"))
	TEnumAsByte<EAblAbilityTargetType> m_ContextActor;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Location", ShortToolTip = "Reference a Location (can use a Context Actor as reference)"))
class UAblParticleEffectParamLocation: public UAblParticleEffectParam
{
	GENERATED_BODY()
public:
	UAblParticleEffectParamLocation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { }
	virtual ~UAblParticleEffectParamLocation() { } 

	FORCEINLINE const FAblAbilityTargetTypeLocation& GetLocation() const { return m_Location; }
private:
	UPROPERTY(EditInstanceOnly, Category = "Parameter", meta = (DisplayName = "Location"))
	FAblAbilityTargetTypeLocation m_Location;
};

#undef LOCTEXT_NAMESPACE