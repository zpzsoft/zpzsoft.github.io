// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Targeting/ablTargetingMisc.h"

#include "ablAbility.h"
#include "ablAbilityContext.h"
#include "Components/PrimitiveComponent.h"

UAblTargetingInstigator::UAblTargetingInstigator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblTargetingInstigator::~UAblTargetingInstigator()
{

}

void UAblTargetingInstigator::FindTargets(UAblAbilityContext& Context) const
{
	if (AActor* InstigatorActor = Context.GetInstigator())
	{
		Context.GetMutableTargetActors().Add(InstigatorActor);
	}
	
	// No need to run filters.
}

UAblTargetingSelf::UAblTargetingSelf(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblTargetingSelf::~UAblTargetingSelf()
{

}

void UAblTargetingSelf::FindTargets(UAblAbilityContext& Context) const
{
	if (AActor* SelfActor = Context.GetSelfActor())
	{
		Context.GetMutableTargetActors().Add(SelfActor);
	}

	// Skip filters.
}

UAblTargetingOwner::UAblTargetingOwner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblTargetingOwner::~UAblTargetingOwner()
{

}

void UAblTargetingOwner::FindTargets(UAblAbilityContext& Context) const
{
	if (AActor* OwnerActor = Context.GetOwner())
	{
		Context.GetMutableTargetActors().Add(OwnerActor);
	}

	// Skip filters.
}
UAblTargetingCustom::UAblTargetingCustom(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblTargetingCustom::~UAblTargetingCustom()
{

}

void UAblTargetingCustom::FindTargets(UAblAbilityContext& Context) const
{
	TArray<AActor*> FoundTargets;
	Context.GetAbility()->CustomTargetingFindTargets(&Context, FoundTargets);
	Context.GetMutableTargetActors().Append(FoundTargets);

	// Run any extra filters.
	FilterTargets(Context);
}
