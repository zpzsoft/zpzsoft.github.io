// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "ablAbilityComponent.h"

#include "ablAbility.h"
#include "ablAbilityInstance.h"
#include "ablAbilityUtilities.h"
#include "AbleCorePrivate.h"
#include "ablSettings.h"
#include "Animation/AnimNode_AbilityAnimPlayer.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Misc/ScopeLock.h"

#if WITH_EDITOR
#include "FXSystem.h"
#endif

#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "AbleCore"

FAblAbilityCooldown::FAblAbilityCooldown()
	: Ability(),
	CurrentTime(0.0f),
	CooldownTime(1.0f)
{

}
FAblAbilityCooldown::FAblAbilityCooldown(const UAblAbility& InAbility, const UAblAbilityContext& Context)
	: Ability(),
	CurrentTime(0.0f),
	CooldownTime(1.0f)
{
	Ability = &InAbility;
	CooldownTime = Ability->CalculateCooldown(&Context);
}

UAblAbilityComponent::UAblAbilityComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_ActiveAbilityInstance(nullptr),
	m_AbilityAnimationNode(nullptr)
{
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bReplicates = true;

	m_Settings = GetDefault<UAbleSettings>(UAbleSettings::StaticClass());
}

void UAblAbilityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AblAbilityComponent::TickComponent"), STAT_AblAbilityComponent_TickComponent, STATGROUP_Able);

	m_IsProcessingUpdate = true;

	// Do our cooldowns first (if we're Async this will give it a bit of time to go ahead and start running).
	if (m_ActiveCooldowns.Num() > 0)
	{
		// Go ahead and fire off our cooldown update first.
		if (UAbleSettings::IsAsyncEnabled() && m_Settings->GetAllowAsyncCooldownUpdate())
		{
			TGraphTask<FAsyncAbilityCooldownUpdaterTask>::CreateTask().ConstructAndDispatchWhenReady(this, DeltaTime);
		}
		else
		{
			UpdateCooldowns(DeltaTime);
		}
	}

	bool ActiveChanged = false;
	bool PassivesChanged = false;

	// Check the status of our Active, we only do this on our authoritative client, or if we're locally controlled for local simulation purposes.
	if (m_ActiveAbilityInstance && 
		(IsAuthoritative() || IsOwnerLocallyControlled()))
	{
		// Check for done...
		if (m_ActiveAbilityInstance->IsDone())
		{
			m_ActiveAbilityInstance->FinishAbility();
			m_ActiveAbilityInstance = nullptr;

			ActiveChanged = true;
		}
		
		// Check for Channeling...
		if (!ActiveChanged && m_ActiveAbilityInstance->IsChanneled())
		{
			if (!m_ActiveAbilityInstance->CheckChannelConditions())
			{
				CancelAbility(&m_ActiveAbilityInstance->GetAbility(), m_ActiveAbilityInstance->GetChannelFailureResult());

				m_ActiveAbilityInstance = nullptr;

				ActiveChanged = true;
			}
		}
	}
	int32 PrePendingPassiveCount = m_PassiveAbilityInstances.Num();
	UAblAbilityInstance*PrePendingActiveInstance = m_ActiveAbilityInstance;

	// Handle any Pending abilities.
	HandlePendingContexts();

	ActiveChanged |= PrePendingActiveInstance != m_ActiveAbilityInstance;
	PassivesChanged |= PrePendingPassiveCount != m_PassiveAbilityInstances.Num();

	// Try and process our Async targeting queue..
	TArray<UAblAbilityContext*> RemovalList;
	for (auto ItAsync = m_AsyncContexts.CreateIterator(); ItAsync; ++ItAsync)
	{
		EAblAbilityStartResult StartResult = InternalStartAbility((*ItAsync));
		if (StartResult != EAblAbilityStartResult::AsyncProcessing)
		{
			// We got some result (either pass or fail), remove this from further processing.
			RemovalList.Add(*ItAsync);

			if ((*ItAsync)->GetAbility()->IsPassive())
			{
				PassivesChanged = true;
			}
			else
			{
				ActiveChanged = true;
			}
		}
	}

	for (UAblAbilityContext* ToRemove : RemovalList)
	{
		m_AsyncContexts.Remove(ToRemove);
	}

	// Update our Active
	if (m_ActiveAbilityInstance)
	{
		// Process update (or launch a task to do it).
		InternalUpdateAbility(m_ActiveAbilityInstance, DeltaTime * m_ActiveAbilityInstance->GetPlayRate());
	}

	int32 CachedPassiveSize = m_PassiveAbilityInstances.Num();

	// Update Passives
	for (UAblAbilityInstance* Passive : m_PassiveAbilityInstances)
	{
		if (Passive)
		{
			if (Passive->IsDone() && (IsAuthoritative() || IsOwnerLocallyControlled()))
			{
				Passive->FinishAbility();
			}
			else
			{
				InternalUpdateAbility(Passive, DeltaTime * Passive->GetPlayRate());

				if (Passive->IsDone())
				{
					Passive->FinishAbility();
				}
			}
		}
	}

	// Clean up finished passives.
	m_PassiveAbilityInstances.RemoveAll([](const TWeakObjectPtr<UAblAbilityInstance>& Instance)->bool
	{
		return !Instance.IsValid() || Instance->IsDone();
	});

	if (CachedPassiveSize != m_PassiveAbilityInstances.Num())
	{
		PassivesChanged = true;
	}

	if (IsNetworked() && IsAuthoritative())
	{
		// Make sure we keep our client watched fields in sync.
		if (ActiveChanged)
		{
			UpdateServerActiveAbility();
		}

		if (PassivesChanged)
		{
			UpdateServerPassiveAbilities();
		}
	}

	CheckNeedsTick();

	m_IsProcessingUpdate = false;
}

void UAblAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// These two fields are replicated and watched by the client.
	DOREPLIFETIME(UAblAbilityComponent, m_ServerActive);
	DOREPLIFETIME(UAblAbilityComponent, m_ServerPassiveAbilities);
}

bool UAblAbilityComponent::IsNetworked() const
{
	return GetNetMode() != ENetMode::NM_Standalone && GetOwnerRole() != ROLE_SimulatedProxy;
}

EAblAbilityStartResult UAblAbilityComponent::CanActivateAbility(UAblAbilityContext* Context) const
{
	if (!Context)
	{
		UE_LOG(LogAble, Warning, TEXT("Null Context passed CanActivateAbility method."));
		return EAblAbilityStartResult::InternalSystemsError;
	}
	
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AblAbilityComponent::CanActivateAbility"), STAT_AblAbilityComponent_CanActivateAbility, STATGROUP_Able);
	
	if (const UAblAbility* Ability = Context->GetAbility())
	{
		if (IsAbilityOnCooldown(Ability))
		{
			return EAblAbilityStartResult::CooldownNotExpired;
		}

		if (m_ActiveAbilityInstance)
		{
			// We're already playing an active ability, check if interrupt is allowed.
			if (!Ability->CanInterruptAbility(Context, &m_ActiveAbilityInstance->GetAbility()))
			{
				return EAblAbilityStartResult::CannotInterruptCurrentAbility;
			}		
		}

		return Ability->CanAbilityExecute(*Context);
	}

	// Our Ability wasn't valid. 
	return EAblAbilityStartResult::InternalSystemsError;
}

EAblAbilityStartResult UAblAbilityComponent::ActivateAbility(UAblAbilityContext* Context)
{
	EAblAbilityStartResult Result = EAblAbilityStartResult::InternalSystemsError;
	if (IsNetworked())
	{
		if (!IsAuthoritative())
		{
			// Pass it to the server to validate/execute.
			ServerActivateAbility(FAblAbilityNetworkContext(*Context));

			// If we're a locally controlled player...
			if (IsOwnerLocallyControlled())
			{
				if (IsProcessingUpdate()) // We're in the middle of an update, set it as the pending context so our Ability has a chance to finish.
				{
					QueueContext(Context, EAblAbilityTaskResult::Successful);

					Result = EAblAbilityStartResult::Success;
				}
				else
				{
					// Go ahead and attempt to play the ability locally, the server is authoritative still. Worst case you end up playing an ability but
					// the server rejects it for whatever reason. That's still far better than having a delay to all actions.
					Result = InternalStartAbility(Context);
				}

			}
			else
			{
				Result = EAblAbilityStartResult::ForwardedToServer;
			}
		}
		else
		{
			// We're authoritative. Start the Ability.
			Result = InternalStartAbility(Context);
		}
	}
	else
	{
		if (IsProcessingUpdate()) // We're in the middle of an update, set it as the pending context so our Ability has a chance to finish.
		{
			QueueContext(Context, EAblAbilityTaskResult::Successful);

			Result = EAblAbilityStartResult::Success;
		}
		else
		{
			// Local game, just pass it along.
			Result = InternalStartAbility(Context);
		}
	}

	CheckNeedsTick();

	return Result;
}

bool UAblAbilityComponent::HasAbilityAnimation() const
{
	return m_AbilityAnimationNode && m_AbilityAnimationNode->HasAnimationToPlay();
}

void UAblAbilityComponent::CancelActiveAbility(EAblAbilityTaskResult ResultToUse)
{
	if (m_ActiveAbilityInstance)
	{
		// Network forwarding should already be handled by this point by CancelAbility.
		if (IsAuthoritative() || IsOwnerLocallyControlled())
		{
			switch (ResultToUse)
			{
			case EAblAbilityTaskResult::Branched:
				m_ActiveAbilityInstance->BranchAbility();
				break;
			case EAblAbilityTaskResult::Interrupted:
				m_ActiveAbilityInstance->InterruptAbility();
				break;
			default:
			case EAblAbilityTaskResult::Successful:
				m_ActiveAbilityInstance->FinishAbility();
				break;
			}
		}

		m_ActiveAbilityInstance = nullptr;
	}

	if (IsNetworked() && IsAuthoritative())
	{
		// Tell the client to reset as well.
		UpdateServerActiveAbility();
	}
}

const UAblAbility* UAblAbilityComponent::GetConstActiveAbility() const
{
	if (m_ActiveAbilityInstance)
	{
		const UAblAbility& CurrentAbility = m_ActiveAbilityInstance->GetAbility();
		return &CurrentAbility;
	}

	return nullptr;
}

UAblAbility* UAblAbilityComponent::GetActiveAbility() const
{
	return const_cast<UAblAbility*>(GetConstActiveAbility());
}

EAblAbilityStartResult UAblAbilityComponent::CanActivatePassiveAbility(UAblAbilityContext* Context) const
{
	if (!Context)
	{
		UE_LOG(LogAble, Warning, TEXT("Null Context passed to CanActivatePassiveAbility method."));
		return EAblAbilityStartResult::InternalSystemsError;
	}

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AblAbilityComponent::CanActivatePassiveAbility"), STAT_AblAbilityComponent_CanActivatePassiveAbility, STATGROUP_Able);

	EAblAbilityStartResult Result = EAblAbilityStartResult::InternalSystemsError;

	if (const UAblAbility* Ability = Context->GetAbility())
	{
		if (IsAbilityOnCooldown(Ability))
		{
			return EAblAbilityStartResult::CooldownNotExpired;
		}

		if (!Ability->IsPassive())
		{
			return EAblAbilityStartResult::NotAllowedAsPassive;
		}

		// Do our stack count first, since it should be pretty cheap.
		int32 CurrentStackCount = GetCurrentStackCountForPassiveAbility(Ability);
		if (CurrentStackCount == 0 || CurrentStackCount < Ability->GetMaxStacks(Context))
		{
			return Ability->CanAbilityExecute(*Context);
		}
		else
		{
			return EAblAbilityStartResult::PassiveMaxStacksReached;
		}
	}

	return Result;
}

EAblAbilityStartResult UAblAbilityComponent::ActivatePassiveAbility(UAblAbilityContext* Context)
{
	if (!Context)
	{
		UE_LOG(LogAble, Warning, TEXT("Null Context passed to ActivatePassiveAbility method."));
		return EAblAbilityStartResult::InternalSystemsError;
	}

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AblAbilityComponent::ActivatePassiveAbility"), STAT_AblAbilityComponent_ActivatePassiveAbility, STATGROUP_Able);

	EAblAbilityStartResult Result = CanActivatePassiveAbility(Context);
	const UAblAbility* Ability = Context->GetAbility();

	if (Result == EAblAbilityStartResult::AsyncProcessing)
	{
		// Save and process it later once the Async query is done.
		m_AsyncContexts.AddUnique(Context);
		return Result;
	}
	else if (Result == EAblAbilityStartResult::PassiveMaxStacksReached)
	{
		if (Ability->AlwaysRefreshDuration())
		{
			UAblAbilityInstance** FoundPassive = m_PassiveAbilityInstances.FindByPredicate(FAblFindAbilityInstanceByHash(Ability->GetAbilityNameHash()));
			if (FoundPassive)
			{
				(*FoundPassive)->ResetTime(Ability->RefreshLoopTimeOnly());

				return EAblAbilityStartResult::Success;
			}
		}
	}
	else if (Result == EAblAbilityStartResult::Success)
	{
		UAblAbilityInstance** FoundPassive = m_PassiveAbilityInstances.FindByPredicate(FAblFindAbilityInstanceByHash(Ability->GetAbilityNameHash()));
		if (FoundPassive)
		{
			UAblAbilityContext& MutableContext = (*FoundPassive)->GetMutableContext();
			MutableContext.SetStackCount(MutableContext.GetCurrentStackCount() + 1);

			if (Ability->RefreshDurationOnNewStack())
			{
				(*FoundPassive)->ResetTime(Ability->RefreshLoopTimeOnly());
			}
		}
		else
		{
			// New Instance
			UAblAbilityInstance* NewInstance = NewObject<UAblAbilityInstance>(this);
			NewInstance->Initialize(*Context);

			// We've passed all our checks, go ahead and allocate our Task scratch pads.
			Context->AllocateScratchPads();

			// Go ahead and start our cooldown.
			AddCooldownForAbility(*Ability, *Context);

			m_PassiveAbilityInstances.Add(NewInstance);
		}
	}
	else
	{
		if (m_Settings->GetLogAbilityFailures())
		{
			UE_LOG(LogAble, Warning, TEXT("[%s] Failed to play Passive Ability [%s] due to reason [%s]."), *GetNetworkRoleText().ToString(), *(Context->GetAbility()->GetDisplayName()), *FAbleLogHelper::GetResultEnumAsString(Result));
		}

		return Result;
	}

	CheckNeedsTick();

	return EAblAbilityStartResult::Success;
}


void UAblAbilityComponent::CancelAbility(const UAblAbility* Ability, EAblAbilityTaskResult ResultToUse)
{
	if (!Ability)
	{
		UE_LOG(LogAble, Warning, TEXT("Null Ability passed to CancelAbility method."));
		return;
	}

	if (!IsAuthoritative())
	{
		ServerCancelAbility(Ability->GetAbilityNameHash(), ResultToUse);

		// Fall through and Locally simulate the cancel if we're player controlled.
		if (!IsOwnerLocallyControlled())
		{
			return;
		}
	}

	InternalCancelAbility(Ability, ResultToUse);
}

int32 UAblAbilityComponent::GetCurrentStackCountForPassiveAbility(const UAblAbility* Ability) const
{
	UAblAbilityInstance* const * FoundPassive = m_PassiveAbilityInstances.FindByPredicate(FAblFindAbilityInstanceByHash(Ability->GetAbilityNameHash()));
	if (FoundPassive)
	{
		return (*FoundPassive)->GetStackCount();
	}

	return 0;
}

void UAblAbilityComponent::GetCurrentPassiveAbilities(TArray<UAblAbility*>& OutPassives) const
{
	OutPassives.Empty();
	for (TArray<UAblAbilityInstance*>::TConstIterator ItConst = m_PassiveAbilityInstances.CreateConstIterator(); ItConst; ++ItConst)
	{
		if ((*ItConst))
		{
			const UAblAbility& PassiveAbility = (*ItConst)->GetAbility();
			// This is the devil, but BP doesn't like const pointers.
			OutPassives.Add(const_cast<UAblAbility*>(&PassiveAbility));
		}
	}
}

float UAblAbilityComponent::GetAbilityCooldownRatio(const UAblAbility* Ability) const
{
	if (Ability && m_ActiveCooldowns.Contains(Ability->GetAbilityNameHash()))
	{
		return m_ActiveCooldowns[Ability->GetAbilityNameHash()].getTimeRatio();
	}

	return 0.0f;
}

float UAblAbilityComponent::GetAbilityCooldownTotal(const UAblAbility* Ability) const
{
	if (Ability && m_ActiveCooldowns.Contains(Ability->GetAbilityNameHash()))
	{
		return m_ActiveCooldowns[Ability->GetAbilityNameHash()].GetCooldownTime();
	}

	return 0.0f;
}

void UAblAbilityComponent::QueueContext(UAblAbilityContext* Context, EAblAbilityTaskResult ResultToUse)
{
	if (IsAuthoritative() || IsOwnerLocallyControlled())
	{
		m_PendingContext.Add(Context); 
		m_PendingResult.Add(ResultToUse);

		CheckNeedsTick();
	}
}

void UAblAbilityComponent::AddAdditionTargetsToContext(const TWeakObjectPtr<const UAblAbilityContext>& Context, const TArray<TWeakObjectPtr<AActor>>& AdditionalTargets, bool AllowDuplicates /*= false*/)
{
	check(Context.IsValid());
	const uint32 AbilityNameHash = Context->GetAbility()->GetAbilityNameHash();
	if (!Context->GetAbility()->IsPassive())
	{
		if (m_ActiveAbilityInstance)
		{
			if (m_ActiveAbilityInstance->GetAbilityNameHash() == AbilityNameHash)
			{
				m_ActiveAbilityInstance->AddAdditionalTargets(AdditionalTargets, AllowDuplicates);
			}
		}
	}
	else
	{
		for (UAblAbilityInstance* Passive : m_PassiveAbilityInstances)
		{
			if (Passive)
			{
				if (Passive->GetAbilityNameHash() == AbilityNameHash)
				{
					Passive->AddAdditionalTargets(AdditionalTargets, AllowDuplicates);
					break;
				}
			}
		}
	}
}

void UAblAbilityComponent::CheckNeedsTick()
{
	// We need to tick if we...
	bool NeedsTick = m_ActiveAbilityInstance != nullptr || // Have an active ability...
		m_PassiveAbilityInstances.Num() || // Have any passive abilities...
		m_ActiveCooldowns.Num() || // Have active cooldowns...
		m_AsyncContexts.Num() || // Have Async targeting to process...
		m_PendingContext.Num(); // We have a pending context...

	PrimaryComponentTick.SetTickFunctionEnable(NeedsTick);
}

EAblAbilityStartResult UAblAbilityComponent::InternalStartAbility(UAblAbilityContext* Context)
{
	check(Context);

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AblAbilityComponent::InternalStartAbility"), STAT_AblAbilityComponent_InternalStartAbility, STATGROUP_Able);
	
	EAblAbilityStartResult Result = EAblAbilityStartResult::InvalidParameter;
	if (Context->GetAbility())
	{
		// Hand off for Passive processing.
		if (Context->GetAbility()->IsPassive())
		{
			return ActivatePassiveAbility(Context);
		}

		Result = CanActivateAbility(Context);
		if (Result == EAblAbilityStartResult::AsyncProcessing)
		{
			// Save and process it later once the Async query is done.
			m_AsyncContexts.AddUnique(Context);

			return Result;
		}
	}

	if (Result != EAblAbilityStartResult::Success)
	{
		if (m_Settings->GetLogAbilityFailures())
		{
			UE_LOG(LogAble, Warning, TEXT("[%s] Failed to play Ability [%s] due to reason [%s]."), *GetNetworkRoleText().ToString(), *(Context->GetAbility()->GetDisplayName()), *FAbleLogHelper::GetResultEnumAsString(Result));
		}

		return Result;
	}

	// Interrupt any current abilities.
	if (m_ActiveAbilityInstance)
	{
		if (!m_ActiveAbilityInstance->IsDone())
		{
			if (IsProcessingUpdate())
			{
				// Delay processing till next frame.
				QueueContext(Context, EAblAbilityTaskResult::Interrupted);
				return Result;
			}

			m_ActiveAbilityInstance->InterruptAbility();
		}
		else
		{
			if (IsProcessingUpdate())
			{
				// Delay processing till next frame.
				QueueContext(Context, EAblAbilityTaskResult::Successful);
				return Result;
			}

			m_ActiveAbilityInstance->FinishAbility();
		}

		m_ActiveAbilityInstance = nullptr;
	}

	UAblAbilityInstance* NewInstance = NewObject<UAblAbilityInstance>(this);
	NewInstance->Initialize(*Context);

	// We've passed all our checks, go ahead and allocate our Task scratch pads.
	Context->AllocateScratchPads();

	// Go ahead and start our cooldown.
	AddCooldownForAbility(*(Context->GetAbility()), *Context);

	m_ActiveAbilityInstance = NewInstance;

	if (IsNetworked() && IsAuthoritative())
	{
		// Propagate changes to client.
		UpdateServerActiveAbility();
	}

	CheckNeedsTick();

	return Result;
}

void UAblAbilityComponent::InternalCancelAbility(const UAblAbility* Ability, EAblAbilityTaskResult ResultToUse)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AblAbilityComponent::InternalCancelAbility"), STAT_AblAbilityComponent_InternalCancelAbility, STATGROUP_Able);

	if (Ability == GetActiveAbility())
	{
		CancelActiveAbility(ResultToUse);
	}
	else if (Ability && Ability->IsPassive())
	{
		for (int i = 0; i < m_PassiveAbilityInstances.Num(); ++i)
		{
			if (m_PassiveAbilityInstances[i]->GetAbilityNameHash() == Ability->GetAbilityNameHash())
			{
				m_PassiveAbilityInstances[i]->FinishAbility();

				m_PassiveAbilityInstances.RemoveAt(i);
				break;
			}
		}
	}
}

void UAblAbilityComponent::AddCooldownForAbility(const UAblAbility& Ability, const UAblAbilityContext& Context)
{
	if (Ability.GetCooldown() > 0.0f)
	{
		m_ActiveCooldowns.Add(Ability.GetAbilityNameHash(), FAblAbilityCooldown(Ability, Context));
	}
}

bool UAblAbilityComponent::IsAbilityOnCooldown(const UAblAbility* Ability) const
{
	return Ability ? m_ActiveCooldowns.Find(Ability->GetAbilityNameHash()) != nullptr : false;
}

bool UAblAbilityComponent::IsPassiveActive(const UAblAbility* Ability) const
{
	if (Ability && Ability->IsPassive())
	{
		return m_PassiveAbilityInstances.FindByPredicate(FAblFindAbilityInstanceByHash(Ability->GetAbilityNameHash())) != nullptr;
	}

	return false;
}

void UAblAbilityComponent::InternalUpdateAbility(UAblAbilityInstance* AbilityInstance, float DeltaTime)
{
	if (AbilityInstance)
	{
		AbilityInstance->PreUpdate();

		if (AbilityInstance->HasAsyncTasks())
		{
			if (UAbleSettings::IsAsyncEnabled() && m_Settings->GetAllowAbilityAsyncUpdate())
			{
				TGraphTask<FAsyncAbilityInstanceUpdaterTask>::CreateTask().ConstructAndDispatchWhenReady(AbilityInstance, AbilityInstance->GetCurrentTime(), DeltaTime);
			}
			else
			{
				// Run our Async update synchronously.
				AbilityInstance->AsyncUpdate(AbilityInstance->GetCurrentTime(), DeltaTime);
			}
		}

		AbilityInstance->SyncUpdate(DeltaTime);
	}
}

void UAblAbilityComponent::UpdateCooldowns(float DeltaTime)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("AblAbilityComponent::UpdateCooldowns"), STAT_AblAbilityComponent_UpdateCooldowns, STATGROUP_Able);
	for (auto ItUpdate = m_ActiveCooldowns.CreateIterator(); ItUpdate; ++ItUpdate)
	{
		FAblAbilityCooldown& AbilityCooldown = ItUpdate->Value;

		AbilityCooldown.Update(DeltaTime);
		if (AbilityCooldown.IsComplete())
		{
			ItUpdate.RemoveCurrent();
		}
	}
}

void UAblAbilityComponent::HandlePendingContexts()
{
	check(m_PendingContext.Num() == m_PendingResult.Num());

	for (int i = 0; i < m_PendingContext.Num(); ++i)
	{
		if (const UAblAbility* PendingAbility = m_PendingContext[i]->GetAbility())
		{
			if (!PendingAbility->IsPassive())
			{
				// If we're about to start an Active ability, we need to first cancel any current active.
				if (m_ActiveAbilityInstance)
				{
					switch (m_PendingResult[i].GetValue())
					{
					case EAblAbilityTaskResult::Interrupted:
						m_ActiveAbilityInstance->InterruptAbility();
						break;
					case EAblAbilityTaskResult::Successful:
						m_ActiveAbilityInstance->FinishAbility();
						break;
					case EAblAbilityTaskResult::Branched:
						m_ActiveAbilityInstance->BranchAbility();
						break;
					}

					m_ActiveAbilityInstance = nullptr;
				}
			}

			InternalStartAbility(m_PendingContext[i]);
		}
	}

	m_PendingContext.Empty();
	m_PendingResult.Empty();
}

FText& UAblAbilityComponent::GetNetworkRoleText() const
{
	static FText ServerText = LOCTEXT("ServerLabel", "Server");
	static FText ClientText = LOCTEXT("ClientLabel", "Client");

	// Not sure if this is true for Autonomous Proxies...
	if (GetOwnerRole() == ROLE_Authority)
	{
		return ServerText;
	}

	return ClientText;
}

bool UAblAbilityComponent::IsOwnerLocallyControlled() const
{
	if (const APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		return PawnOwner->IsLocallyControlled();
	}

	return false;
}

bool UAblAbilityComponent::IsAuthoritative() const
{
	return !IsNetworked() || (IsNetworked() && GetOwnerRole() == ROLE_Authority);
}

EAblAbilityStartResult UAblAbilityComponent::BranchAbility(UAblAbilityContext* Context)
{
	EAblAbilityStartResult Result = EAblAbilityStartResult::InternalSystemsError;
	if (!IsAuthoritative())
	{
		ServerBranchAbility(FAblAbilityNetworkContext(*Context));

		if (IsOwnerLocallyControlled())
		{
			QueueContext(Context, EAblAbilityTaskResult::Branched);
			Result = EAblAbilityStartResult::Success;
		}
		else
		{
			Result = EAblAbilityStartResult::ForwardedToServer;
		}
	}
	else
	{
		QueueContext(Context, EAblAbilityTaskResult::Branched);
		Result = EAblAbilityStartResult::Success;
	}

	CheckNeedsTick();

	return Result;
}

bool UAblAbilityComponent::IsAbilityRealmAllowed(const UAblAbility& Ability) const
{
	if (!IsNetworked())
	{
		return true;
	}

	switch (Ability.GetAbilityRealm())
	{
		case EAblAbilityTaskRealm::Client:
		{
			return !IsAuthoritative() || IsOwnerLocallyControlled() || GetWorld()->GetNetMode() == ENetMode::NM_ListenServer;
		}
		break;
		case EAblAbilityTaskRealm::Server:
		{
			return IsAuthoritative();
		}
		break;
		case EAblAbilityTaskRealm::ClientAndServer:
		{
			return true;
		}
		break;
		default:
			break;
	}

	return false;
}

void UAblAbilityComponent::UpdateServerPassiveAbilities()
{
	check(IsAuthoritative()); // Should only be called on the server.

	TArray<uint32> Whitelist;
	for (UAblAbilityInstance* PassiveInstance : m_PassiveAbilityInstances)
	{
		Whitelist.Add(PassiveInstance->GetAbilityNameHash());
		if (FAblAbilityNetworkContext* ExistingNetworkContext = m_ServerPassiveAbilities.FindByPredicate(FAblFindAbilityNetworkContextByHash(PassiveInstance->GetAbilityNameHash())))
		{
			ExistingNetworkContext->SetCurrentStacks(PassiveInstance->GetStackCount());
		}
		else
		{
			// New Passive.
			m_ServerPassiveAbilities.Add(FAblAbilityNetworkContext(PassiveInstance->GetContext()));
		}
	}

	m_ServerPassiveAbilities.RemoveAll(FAblAbilityNetworkContextWhiteList(Whitelist));
}

void UAblAbilityComponent::UpdateServerActiveAbility()
{
	check(IsAuthoritative()); // Should only be called on the server.

	if (!m_ActiveAbilityInstance)
	{
		m_ServerActive.Reset();
	}
	else if(!m_ServerActive.IsValid() || m_ServerActive.GetAbility()->GetAbilityNameHash() != m_ActiveAbilityInstance->GetAbilityNameHash())
	{
		m_ServerActive = FAblAbilityNetworkContext(m_ActiveAbilityInstance->GetContext());
	}
}

void UAblAbilityComponent::ServerActivateAbility_Implementation(const FAblAbilityNetworkContext& Context)
{
	UAblAbilityContext* LocalContext = UAblAbilityContext::MakeContext(Context);
	if (InternalStartAbility(LocalContext) == EAblAbilityStartResult::Success)
	{
		if (!Context.GetAbility()->IsPassive())
		{
			m_ServerActive = Context;
		}
		else
		{
			int32 StackCount = GetCurrentStackCountForPassiveAbility(Context.GetAbility().Get());
			if (FAblAbilityNetworkContext* ExistingContext = m_ServerPassiveAbilities.FindByPredicate(FAblFindAbilityNetworkContextByHash(Context.GetAbility()->GetAbilityNameHash())))
			{
				ExistingContext->SetCurrentStacks(StackCount);
			}
			else
			{
				FAblAbilityNetworkContext NewNetworkContext(*LocalContext);
				NewNetworkContext.SetCurrentStacks(StackCount);
				m_ServerPassiveAbilities.Add(NewNetworkContext);
			}
		}
	}
}

bool UAblAbilityComponent::ServerActivateAbility_Validate(const FAblAbilityNetworkContext& Context)
{
	// Nothing really to check here since the server isn't trusting the Client at all.
	return Context.IsValid();
}

void UAblAbilityComponent::ServerCancelAbility_Implementation(uint32 AbilityNameHash, EAblAbilityTaskResult ResultToUse)
{
	const UAblAbility* Ability = nullptr;
	if (m_ActiveAbilityInstance && m_ActiveAbilityInstance->GetAbilityNameHash() == AbilityNameHash)
	{
		Ability = &m_ActiveAbilityInstance->GetAbility();
	}
	else
	{
		if (UAblAbilityInstance** PassiveInstance = m_PassiveAbilityInstances.FindByPredicate(FAblFindAbilityInstanceByHash(AbilityNameHash)))
		{
			Ability = &(*PassiveInstance)->GetAbility();
		}
	}

	if (Ability)
	{
		// Just pass it along.
		InternalCancelAbility(Ability, ResultToUse);
	}

}

bool UAblAbilityComponent::ServerCancelAbility_Validate(uint32 AbilityNameHash, EAblAbilityTaskResult ResultToUse)
{
	return AbilityNameHash != 0U;
}

void UAblAbilityComponent::ServerBranchAbility_Implementation(const FAblAbilityNetworkContext& Context)
{
	check(IsAuthoritative());
	QueueContext(UAblAbilityContext::MakeContext(Context), EAblAbilityTaskResult::Branched);
}

bool UAblAbilityComponent::ServerBranchAbility_Validate(const FAblAbilityNetworkContext& Context)
{
	return Context.IsValid();
}

void UAblAbilityComponent::OnServerActiveAbilityChanged()
{
	if (m_ServerActive.IsValid())
	{
		if (IsPlayingAbility() && 
			IsOwnerLocallyControlled() && 
			m_ActiveAbilityInstance->GetAbility().GetAbilityNameHash() == m_ServerActive.GetAbility()->GetAbilityNameHash())
		{
			// Same Ability, skip it since we were locally predicting it.
			return;
		}

		if (IsPlayingAbility())
		{
			// TODO: Should the client care about server based interrupt/branches?
			InternalCancelAbility(GetActiveAbility(), EAblAbilityTaskResult::Successful);
		}

		InternalStartAbility(UAblAbilityContext::MakeContext(m_ServerActive));
	}
	else // Server Ability was reset, make sure we finish our ability.
	{
		if (IsPlayingAbility())
		{
			// TODO: Should the client care about server based interrupt/branches?
			InternalCancelAbility(GetActiveAbility(), EAblAbilityTaskResult::Successful);
		}
	}
}

void UAblAbilityComponent::OnServerPassiveAbilitiesChanged()
{
	TArray<uint32> ValidAbilityNameHashes;
	for (const FAblAbilityNetworkContext& ServerPassive : m_ServerPassiveAbilities)
	{
		if (ServerPassive.IsValid() && ServerPassive.GetAbility().IsValid())
		{
			if (UAblAbilityInstance** CurrentPassive = m_PassiveAbilityInstances.FindByPredicate(FAblFindAbilityInstanceByHash(ServerPassive.GetAbility()->GetAbilityNameHash())))
			{
				// Just make sure our stack count is accurate.
				(*CurrentPassive)->SetStackCount(ServerPassive.GetCurrentStack());
			}
			else // New Passive Ability
			{
				ActivatePassiveAbility(UAblAbilityContext::MakeContext(ServerPassive));
			}

			ValidAbilityNameHashes.Add(ServerPassive.GetAbility()->GetAbilityNameHash());
		}
	}

	m_PassiveAbilityInstances.RemoveAll(FAblAbilityInstanceWhiteList(ValidAbilityNameHashes));
}

#if WITH_EDITOR

void UAblAbilityComponent::PlayAbilityFromEditor(const UAblAbility* Ability)
{
	// Interrupt any current abilities.
	if (m_ActiveAbilityInstance)
	{
		m_ActiveAbilityInstance->StopAbility();
		m_ActiveAbilityInstance = nullptr;
	}

	if (!Ability)
	{
		return;
	}

	UAblAbilityContext* FakeContext = UAblAbilityContext::MakeContext(Ability, this, GetOwner(), nullptr);
	// Run Targeting so we can visually see it - we don't care about the result since the editor always succeeds.
	Ability->CanAbilityExecute(*FakeContext);

	UAblAbilityInstance* NewInstance = NewObject<UAblAbilityInstance>(this);
	NewInstance->Initialize(*FakeContext);

	// We've passed all our checks, go ahead and allocate our Task scratch pads.
	FakeContext->AllocateScratchPads();

	m_ActiveAbilityInstance = NewInstance;

	CheckNeedsTick();
}

float UAblAbilityComponent::GetCurrentAbilityTime() const
{
	if (m_ActiveAbilityInstance)
	{
		return m_ActiveAbilityInstance->GetCurrentTime();
	}

	return 0.0f;
}

void UAblAbilityComponent::SetAbilityTime(float NewTime)
{
	if (m_ActiveAbilityInstance)
	{
		m_ActiveAbilityInstance->SetCurrentTime(NewTime);
	}
}

#endif


void UAblAbilityComponent::AddTag(const FGameplayTag Tag)
{
	m_TagContainer.AddTag(Tag);
}

void UAblAbilityComponent::RemoveTag(const FGameplayTag Tag)
{
	m_TagContainer.RemoveTag(Tag);
}

bool UAblAbilityComponent::HasTag(const FGameplayTag Tag) const
{
	return m_TagContainer.HasTag(Tag);
}

bool UAblAbilityComponent::MatchesAnyTag(const FGameplayTagContainer Container) const
{
	return m_TagContainer.HasAny(Container);
}

bool UAblAbilityComponent::MatchesAllTags(const FGameplayTagContainer Container) const
{
	return m_TagContainer.HasAll(Container);
}

bool UAblAbilityComponent::MatchesQuery(const FGameplayTagQuery Query) const
{
	return m_TagContainer.MatchesQuery(Query);
}

void UAblAbilityComponent::SetAbilityAnimationNode(const FAnimNode_AbilityAnimPlayer* Node)
{
	FScopeLock CS(&m_AbilityAnimNodeCS);

	m_AbilityAnimationNode = Node;
}

#undef LOCTEXT_NAMESPACE

