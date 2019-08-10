// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Channeling/ablChannelingConditions.h"

#include "ablAbility.h"
#include "ablAbilityContext.h"
#include "ablAbilityUtilities.h"
#include "AbleCorePrivate.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

UAblChannelingInputConditional::UAblChannelingInputConditional(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblChannelingInputConditional::~UAblChannelingInputConditional()
{

}

bool UAblChannelingInputConditional::CheckConditional(UAblAbilityContext& Context) const
{
	if (UAblAbilityComponent* AbilityComponent = Context.GetSelfAbilityComponent())
	{
		if (AbilityComponent->IsNetworked() && !AbilityComponent->IsOwnerLocallyControlled())
		{
			// We can only check Input on local clients (due to keybindings, etc), assume its valid unless Client tells us otherwise.
			return true;
		}
	}

	if (!m_InputKeyCache.Num())
	{
		for (const FName& InputAction : m_InputActions)
		{
			m_InputKeyCache.Append(FAblAbilityUtilities::GetKeysForInputAction(InputAction));
		}
	}

	if (const AActor* SelfActor = Context.GetSelfActor())
	{
		if (const APawn* Pawn = Cast<APawn>(SelfActor))
		{
			if (const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
			{
				for (const FKey& Key : m_InputKeyCache)
				{
					if (PlayerController->IsInputKeyDown(Key))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

UAblChannelingVelocityConditional::UAblChannelingVelocityConditional(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_VelocityThreshold(1.0f),
	m_UseXYSpeed(true)
{

}

UAblChannelingVelocityConditional::~UAblChannelingVelocityConditional()
{

}

bool UAblChannelingVelocityConditional::CheckConditional(UAblAbilityContext& Context) const
{
	if (const AActor* SelfActor = Context.GetSelfActor())
	{
		const FVector Velocity = SelfActor->GetVelocity();
		const float ThresholdSquared = m_VelocityThreshold * m_VelocityThreshold;

		if (m_UseXYSpeed)
		{
			return Velocity.SizeSquared2D() < ThresholdSquared;
		}
		else
		{
			return Velocity.SizeSquared() < ThresholdSquared;
		}
	}

	return false;
}

UAblChannelingCustomConditional::UAblChannelingCustomConditional(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_EventName(NAME_None)
{

}

UAblChannelingCustomConditional::~UAblChannelingCustomConditional()
{

}

bool UAblChannelingCustomConditional::CheckConditional(UAblAbilityContext& Context) const
{
	check(Context.GetAbility() != nullptr);
	return Context.GetAbility()->CheckCustomChannelConditional(&Context, m_EventName);
}
