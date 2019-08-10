// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AI/BTTask_StopAbility.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_StopAbility::UBTTask_StopAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	CancelActive(true),
	ResultToUse(EAblAbilityTaskResult::Successful)
{
	NodeName = "Stop Ability";
}

EBTNodeResult::Type UBTTask_StopAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const OwnerController = OwnerComp.GetAIOwner();
	const UAblAbility* AbilityCDO = Cast<UAblAbility>(Ability.GetDefaultObject());
	if (OwnerController)
	{
		if (APawn* Owner = OwnerController->GetPawn())
		{
			if (UAblAbilityComponent* AbilityComponent = Owner->FindComponentByClass<UAblAbilityComponent>())
			{
				AbilityComponent->CancelAbility(CancelActive ? AbilityComponent->GetActiveAbility() : AbilityCDO, ResultToUse.GetValue());
				
				return EBTNodeResult::Succeeded;		
			}
		}
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_StopAbility::GetStaticDescription() const
{
	FString AbilityName = TEXT("Current Active"); 
	
	if (!CancelActive)
	{
		AbilityName = Ability.GetDefaultObject() ? Ability.GetDefaultObject()->GetDisplayName() : TEXT("Invalid");
	}

	return FString::Printf(TEXT("%s: Stops the [%s] Ability."), *Super::GetStaticDescription(), *AbilityName);
}

#if WITH_EDITOR
FName UBTTask_StopAbility::GetNodeIconName() const
{
	return Super::GetNodeIconName();
}
#endif
