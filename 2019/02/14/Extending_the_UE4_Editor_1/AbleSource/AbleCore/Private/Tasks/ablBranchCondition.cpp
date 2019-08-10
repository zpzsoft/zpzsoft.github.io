// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablBranchCondition.h"

#include "ablAbility.h"
#include "ablAbilityUtilities.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Tasks/ablBranchTask.h"

UAblBranchCondition::UAblBranchCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Negate(false)
{

}

UAblBranchCondition::~UAblBranchCondition()
{

}

UAblBranchConditionOnInput::UAblBranchConditionOnInput(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_InputActions(),
	m_MustBeRecentlyPressed(false),
	m_RecentlyPressedTimeLimit(0.1f)
{

}

UAblBranchConditionOnInput::~UAblBranchConditionOnInput()
{

}

bool UAblBranchConditionOnInput::CheckCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context, UAblBranchTaskScratchPad& ScratchPad) const
{
	// Build our cache
	if (ScratchPad.CachedKeys.Num() == 0)
	{
		for (const FName& Action : m_InputActions)
		{
			ScratchPad.CachedKeys.Append(FAblAbilityUtilities::GetKeysForInputAction(Action));
		}
	}

	if (UAblAbilityComponent* AbilityComponent = Context->GetSelfAbilityComponent())
	{
		if (AbilityComponent->IsNetworked() && !AbilityComponent->IsOwnerLocallyControlled())
		{
			// We can only check Input on local clients.
			return false;
		}
	}

	if (const APawn* Pawn = Cast<APawn>(Context->GetSelfActor()))
	{
		if (const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			for (const FKey& Key : ScratchPad.CachedKeys)
			{
				if (PlayerController->IsInputKeyDown(Key))
				{
					if (m_MustBeRecentlyPressed)
					{
						return PlayerController->GetInputKeyTimeDown(Key) <= m_RecentlyPressedTimeLimit;
					}
					else
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

UAblBranchConditionAlways::UAblBranchConditionAlways(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblBranchConditionAlways::~UAblBranchConditionAlways()
{

}

UAblBranchConditionCustom::UAblBranchConditionCustom(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblBranchConditionCustom::~UAblBranchConditionCustom()
{

}

bool UAblBranchConditionCustom::CheckCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context, UAblBranchTaskScratchPad& ScratchPad) const
{
	check(Context.IsValid() && ScratchPad.BranchAbility);

	return Context.Get()->GetAbility()->CustomCanBranchTo(Context.Get(), ScratchPad.BranchAbility);
}
