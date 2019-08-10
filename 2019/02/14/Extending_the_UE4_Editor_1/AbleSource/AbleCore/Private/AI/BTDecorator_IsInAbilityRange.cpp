// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "AI/BTDecorator_IsInAbilityRange.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

UBTDecorator_IsInAbilityRange::UBTDecorator_IsInAbilityRange(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Is In Ability Range";

	PointA.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsInAbilityRange, PointA), AActor::StaticClass());
	PointA.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsInAbilityRange, PointA));

	PointB.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsInAbilityRange, PointB), AActor::StaticClass());
	PointB.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsInAbilityRange, PointB));

	// Default to using Self Actor
	PointA.SelectedKeyName = FBlackboard::KeySelf;

	// For now, don't allow users to select any "Abort Observers", because it's currently not supported.
	bAllowAbortNone = false;
	bAllowAbortLowerPri = false;
	bAllowAbortChildNodes = false;
}

bool UBTDecorator_IsInAbilityRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr || Ability.GetDefaultObject() == nullptr)
	{
		return false;
	}

	if (const UAblAbility* AbilityCDO = Cast<UAblAbility>(Ability.GetDefaultObject()))
	{
		const float AbilityRange = AbilityCDO->GetRange();
		const float AbilityRangeSqr = AbilityRange * AbilityRange;

		if (AbilityRange < KINDA_SMALL_NUMBER)
		{
			// No range, allow it.
			return true;
		}

		FVector Origin, Destination;
		if (BlackboardComp->GetLocationFromEntry(PointA.GetSelectedKeyID(), Origin))
		{
			if (BlackboardComp->GetLocationFromEntry(PointB.GetSelectedKeyID(), Destination))
			{
				if (XYDistance)
				{
					return FVector::DistSquaredXY(Destination, Origin) < AbilityRangeSqr;
				}
				else
				{
					return FVector::DistSquared(Destination, Origin) < AbilityRangeSqr;
				}
			}
		}
	}

	return false;
}

FString UBTDecorator_IsInAbilityRange::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: checks if the distance between the two provided points is in range of the [%s] Ability."), *Super::GetStaticDescription(),
		Ability.GetDefaultObject() ? *Ability.GetDefaultObject()->GetDisplayName() : TEXT("Invalid"));
}

void UBTDecorator_IsInAbilityRange::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		PointA.ResolveSelectedKey(*BBAsset);
		PointB.ResolveSelectedKey(*BBAsset);
	}
}