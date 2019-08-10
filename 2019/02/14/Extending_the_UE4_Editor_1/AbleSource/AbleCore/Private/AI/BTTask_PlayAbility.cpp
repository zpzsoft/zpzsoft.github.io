// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AI/BTTask_PlayAbility.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_PlayAbility::UBTTask_PlayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	MarkAsInProgressDuringExecution(false)
{
	NodeName = "Play Ability";
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_PlayAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTPlayAbilityTaskMemory* TaskMemory = reinterpret_cast<FBTPlayAbilityTaskMemory*>(NodeMemory);
	AAIController* const OwnerController = OwnerComp.GetAIOwner();
	const UAblAbility* AbilityCDO = Cast<UAblAbility>(Ability.GetDefaultObject());
	if (AbilityCDO && OwnerController)
	{
		if (APawn* Owner = OwnerController->GetPawn())
		{
			if (UAblAbilityComponent* AbilityComponent = Owner->FindComponentByClass<UAblAbilityComponent>())
			{
				if (!TaskMemory->bAbilityPlayed)
				{
					UAblAbilityContext* Context = UAblAbilityContext::MakeContext(AbilityCDO, AbilityComponent, Owner, nullptr);
					EAblAbilityStartResult Result = AbilityComponent->ActivateAbility(Context);
					if (Result == EAblAbilityStartResult::Success || Result == EAblAbilityStartResult::AsyncProcessing)
					{
						TaskMemory->bAbilityPlayed = true;
						if (MarkAsInProgressDuringExecution)
						{
							TaskMemory->AbilityComponent = AbilityComponent;
							return EBTNodeResult::InProgress;
						}
						else
						{
							return EBTNodeResult::Succeeded;
						}
					}
					else
					{
						return EBTNodeResult::Failed;
					}
				}
			}
		}
	}

	return EBTNodeResult::Failed;
}

void UBTTask_PlayAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTPlayAbilityTaskMemory* TaskMemory = reinterpret_cast<FBTPlayAbilityTaskMemory*>(NodeMemory);
	if (MarkAsInProgressDuringExecution && TaskMemory->bAbilityPlayed)
	{
		const UAblAbility* AbilityCDO = Cast<UAblAbility>(Ability.GetDefaultObject());
		if (UAblAbilityComponent* AbilityComponent = TaskMemory->AbilityComponent.Get())
		{
			// Our Active is no longer the Ability we were playing. Finish this Task.
			if (AbilityComponent->GetActiveAbility() != AbilityCDO)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
}

void UBTTask_PlayAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	FBTPlayAbilityTaskMemory* TaskMemory = reinterpret_cast<FBTPlayAbilityTaskMemory*>(NodeMemory);
	
	TaskMemory->bAbilityPlayed = false;
	TaskMemory->AbilityComponent.Reset();

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_PlayAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: Play the [%s] Ability."), *Super::GetStaticDescription(), Ability.GetDefaultObject() ? *Ability.GetDefaultObject()->GetDisplayName() : TEXT("Invalid"));
}

void UBTTask_PlayAbility::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);

	FBTPlayAbilityTaskMemory* TaskMemory = reinterpret_cast<FBTPlayAbilityTaskMemory*>(NodeMemory);

	const FString RuntimeDesc = TaskMemory->bAbilityPlayed ? (MarkAsInProgressDuringExecution ? TEXT("In Progress") : TEXT("Success")) : TEXT("Waiting");

	Values.Add(RuntimeDesc);
}

uint16 UBTTask_PlayAbility::GetInstanceMemorySize() const
{
	return sizeof(FBTPlayAbilityTaskMemory);
}

void UBTTask_PlayAbility::PostLoad()
{
	Super::PostLoad();

	bNotifyTick = MarkAsInProgressDuringExecution;
}

#if WITH_EDITOR
FName UBTTask_PlayAbility::GetNodeIconName() const
{
	return Super::GetNodeIconName();
}
#endif
