// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "ablAbilityContext.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/ScopeLock.h"
#include "Net/UnrealNetwork.h"
#include "Tasks/IAblAbilityTask.h"

FVector FAblQueryResult::GetLocation() const
{
	FTransform TargetTransform;
	GetTransform(TargetTransform);

	return TargetTransform.GetTranslation();
}

void FAblQueryResult::GetTransform(FTransform& OutTransform) const
{
	OutTransform = FTransform::Identity;

	if (Actor.IsValid())
	{
		OutTransform = Actor->GetActorTransform();
	}
	else if (PrimitiveComponent.IsValid())
	{
		if (AActor* ActorOwner = PrimitiveComponent->GetOwner())
		{
			OutTransform = ActorOwner->GetActorTransform();
		}
		else
		{
			// Is this ever really valid?
			OutTransform = PrimitiveComponent->GetComponentTransform();
		}
	}
}

UAblAbilityContext::UAblAbilityContext(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Ability(nullptr),
	m_AbilityComponent(nullptr),
	m_StackCount(1),
	m_CurrentTime(0.0f),
	m_LastDelta(0.0f)
{

}

UAblAbilityContext::~UAblAbilityContext()
{

}

UAblAbilityContext* UAblAbilityContext::MakeContext(const UAblAbility* Ability, UAblAbilityComponent* AbilityComponent, AActor* Owner, AActor* Instigator)
{
	if (!Ability || !AbilityComponent)
	{
		return nullptr;
	}

	UAblAbilityContext* NewContext = NewObject<UAblAbilityContext>(AbilityComponent);
	NewContext->m_Ability = Ability;
	NewContext->m_AbilityComponent = AbilityComponent;

	if (Owner)
	{
		NewContext->m_Owner = Owner;
	}

	if (Instigator)
	{
		NewContext->m_Instigator = Instigator;
	}

	return NewContext;
}

UAblAbilityContext* UAblAbilityContext::MakeContext(const FAblAbilityNetworkContext& NetworkContext)
{
	UAblAbilityContext* NewContext = MakeContext(NetworkContext.GetAbility().Get(), NetworkContext.GetAbilityComponent().Get(), NetworkContext.GetOwner().Get(), NetworkContext.GetInstigator().Get());
	NewContext->GetMutableTargetActors().Append(NetworkContext.GetTargetActors());

	return NewContext;
}

void UAblAbilityContext::AllocateScratchPads()
{
	const TArray<UAblAbilityTask*>& Tasks = m_Ability->GetTasks();

	for (UAblAbilityTask* Task : Tasks)
	{
		if (UAblAbilityTaskScratchPad* ScratchPad = Task->CreateScratchPad(TWeakObjectPtr<UAblAbilityContext>(this)))
		{
			m_TaskScratchPadMap.Add(Task->GetUniqueID(), ScratchPad);
		}
	}
}


void UAblAbilityContext::UpdateTime(float DeltaTime)
{
	FPlatformMisc::MemoryBarrier();
	m_CurrentTime += DeltaTime;
	m_LastDelta = DeltaTime;
}

UAblAbilityTaskScratchPad* UAblAbilityContext::GetScratchPadForTask(const class UAblAbilityTask* Task) const
{
	UAblAbilityTaskScratchPad* const * ScratchPad = m_TaskScratchPadMap.Find(Task->GetUniqueID());
	if (ScratchPad && (*ScratchPad) != nullptr)
	{
		return *ScratchPad;
	}

	return nullptr;
}

UAblAbilityComponent* UAblAbilityContext::GetSelfAbilityComponent() const
{
	return m_AbilityComponent;
}

AActor* UAblAbilityContext::GetSelfActor() const
{
	if (m_AbilityComponent)
	{
		return m_AbilityComponent->GetOwner();
	}

	return nullptr;
}

int32 UAblAbilityContext::GetCurrentStackCount() const
{
	return m_StackCount;
}

void UAblAbilityContext::SetStackCount(int32 Stack)
{
	int32 MaxStacks = GetAbility()->GetMaxStacks(this);
	m_StackCount = FMath::Clamp(Stack, 1, MaxStacks);
}

float UAblAbilityContext::GetCurrentTimeRatio() const
{
	return FMath::Clamp(m_CurrentTime / m_Ability->GetLength(), 0.0f, 1.0f);
}

const TArray<AActor*> UAblAbilityContext::GetTargetActors() const
{
	// Blueprints don't like Weak Ptrs, so we have to do this fun copy.
	TArray<AActor*> ReturnVal;
	ReturnVal.Reserve(m_TargetActors.Num());

	for (const TWeakObjectPtr<AActor>& Actor : m_TargetActors)
	{
		if (Actor.IsValid())
		{
			ReturnVal.Add(Actor.Get());
		}
	}

	return ReturnVal;
}

FAblAbilityNetworkContext::FAblAbilityNetworkContext()
{
	Reset();
}

FAblAbilityNetworkContext::FAblAbilityNetworkContext(const UAblAbilityContext& Context)
	: m_Ability(Context.GetAbility()),
	m_AbilityComponent(Context.GetSelfAbilityComponent()),
	m_Owner(Context.GetOwner()),
	m_Instigator(Context.GetInstigator()),
	m_TargetActors(Context.GetTargetActors()),
	m_CurrentStacks(Context.GetCurrentStackCount())
{
	if (GEngine && GEngine->GetWorld())
	{
		m_TimeStamp = GEngine->GetWorld()->GetRealTimeSeconds();
	}
}

void FAblAbilityNetworkContext::Reset()
{
	m_Ability = nullptr;
	m_AbilityComponent = nullptr;
	m_Owner.Reset();
	m_Instigator.Reset();
	m_TargetActors.Empty();
	m_CurrentStacks = 0;
}

bool FAblAbilityNetworkContext::IsValid() const
{
	return m_Ability != nullptr && m_AbilityComponent != nullptr;
}
