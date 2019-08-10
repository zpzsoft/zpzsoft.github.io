// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AI/BTDecorator_IsAbilityOnCooldown.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

UBTDecorator_IsAbilityOnCooldown::UBTDecorator_IsAbilityOnCooldown(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Ability Cooldown Condition";

	// Accept only actors
	ActorToCheck.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsAbilityOnCooldown, ActorToCheck), AActor::StaticClass());

	// Default to using Self Actor
	ActorToCheck.SelectedKeyName = FBlackboard::KeySelf;

	// For now, don't allow users to select any "Abort Observers", because it's currently not supported.
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

bool UBTDecorator_IsAbilityOnCooldown::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr || Ability.GetDefaultObject() == nullptr)
	{
		return false;
	}

	if (AActor* Actor = Cast<AActor>(BlackboardComp->GetValue<UBlackboardKeyType_Object>(ActorToCheck.GetSelectedKeyID())))
	{
		if (const UAblAbility* AbilityCDO = Cast<UAblAbility>(Ability.GetDefaultObject()))
		{
			if (UAblAbilityComponent* AbilityComponent = Actor->FindComponentByClass<UAblAbilityComponent>())
			{
				return AbilityComponent->IsAbilityOnCooldown(AbilityCDO);
			}
		}
	}

	return false;
}

FString UBTDecorator_IsAbilityOnCooldown::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: checks if the Ability [%s] is on Cooldown."), *Super::GetStaticDescription(), 
		Ability.GetDefaultObject() ? *Ability.GetDefaultObject()->GetDisplayName() : TEXT("Invalid"));
}

void UBTDecorator_IsAbilityOnCooldown::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		ActorToCheck.ResolveSelectedKey(*BBAsset);
	}
}


