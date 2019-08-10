// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AI/BTDecorator_HasActivePassiveAbility.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

UBTDecorator_HasActivePassiveAbility::UBTDecorator_HasActivePassiveAbility(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	NodeName = "Ability Playing Condition";

	// Accept only actors
	ActorToCheck.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_HasActivePassiveAbility, ActorToCheck), AActor::StaticClass());

	// Default to using Self Actor
	ActorToCheck.SelectedKeyName = FBlackboard::KeySelf;

	// For now, don't allow users to select any "Abort Observers", because it's currently not supported.
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

bool UBTDecorator_HasActivePassiveAbility::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr)
	{
		return false;
	}

	if (AActor* Actor = Cast<AActor>(BlackboardComp->GetValue<UBlackboardKeyType_Object>(ActorToCheck.GetSelectedKeyID())))
	{
		if (const UAblAbility* AbilityCDO = Cast<UAblAbility>(Ability.GetDefaultObject()))
		{
			if (UAblAbilityComponent* AbilityComponent = Actor->FindComponentByClass<UAblAbilityComponent>())
			{
				return AbilityComponent->IsPassiveActive(AbilityCDO);
			}
		}
	}

	return false;
}

FString UBTDecorator_HasActivePassiveAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: checks if the actor has the passive Ability [%s] active."), *Super::GetStaticDescription(),
		Ability.GetDefaultObject() ? *Ability.GetDefaultObject()->GetDisplayName() : TEXT("Unknown"));
}

void UBTDecorator_HasActivePassiveAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		ActorToCheck.ResolveSelectedKey(*BBAsset);
	}
}
