// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablSpawnActorTask.h"

#include "ablAbility.h"
#include "ablAbilityContext.h"
#include "AbleCorePrivate.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblSpawnActorTaskScratchPad::UAblSpawnActorTaskScratchPad()
{

}

UAblSpawnActorTaskScratchPad::~UAblSpawnActorTaskScratchPad()
{

}

UAblSpawnActorTask::UAblSpawnActorTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_ActorClass(),
	m_InitialVelocity(ForceInitToZero),
	m_SetOwner(true),
	m_OwnerTargetType(EAblAbilityTargetType::Self),
	m_AttachToOwnerSocket(false),
	m_AttachmentRule(EAttachmentRule::KeepRelative),
	m_SocketName(NAME_None),
	m_InheritOwnerLinearVelocity(false),
	m_MarkAsTransient(true),
	m_DestroyAtEnd(false),
	m_FireEvent(false),
	m_Name(NAME_None)
{

}

UAblSpawnActorTask::~UAblSpawnActorTask()
{

}

void UAblSpawnActorTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	check(Context.IsValid());

	Super::OnTaskStart(Context);

	TArray<TWeakObjectPtr<AActor>> OutActors;
	GetActorsForTask(Context, OutActors);

	if (OutActors.Num() == 0)
	{
		// Just incase, to not break previous content.
		OutActors.Add(Context->GetSelfActor());
	}

	if (!m_ActorClass.GetDefaultObject())
	{
		UE_LOG(LogAble, Warning, TEXT("SpawnActorTask for Ability [%s] does not have a class specified."), *(Context->GetAbility()->GetDisplayName()));
		return;
	}

	UAblSpawnActorTaskScratchPad* ScratchPad = nullptr;
	if (m_DestroyAtEnd)
	{
		ScratchPad = Cast<UAblSpawnActorTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
	}
	
	FTransform SpawnTransform;
	m_SpawnLocation.GetTransform(*Context.Get(), SpawnTransform);

	FVector SpawnLocation;

	for (int32 i = 0; i < OutActors.Num(); ++i)
	{
		UWorld* ActorWorld = OutActors[i]->GetWorld();
		FActorSpawnParameters SpawnParams;

		if (m_SpawnLocation.GetSourceTargetType() == EAblAbilityTargetType::TargetActor)
		{
			m_SpawnLocation.GetTargetTransform(*Context.Get(), i, SpawnTransform);
		}

		if (m_SetOwner)
		{
			SpawnParams.Owner = GetSingleActorFromTargetType(Context, m_OwnerTargetType);
		}

		SpawnParams.Name = MakeUniqueObjectName(ActorWorld, m_ActorClass);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (m_MarkAsTransient)
		{
			SpawnParams.ObjectFlags = EObjectFlags::RF_Transient; // We don't want to save anything on this object.
		}
		
#if !(UE_BUILD_SHIPPING)
		if (IsVerbose())
		{
			PrintVerbose(FString::Printf(TEXT("Spawning Actor %s using Transform %s."), *m_ActorClass->GetName(), *SpawnTransform.ToString()));
		}
#endif

		AActor* SpawnedActor = ActorWorld->SpawnActor<AActor>(m_ActorClass, SpawnTransform, SpawnParams);
		check(SpawnedActor);
		
		FVector InheritedVelocity;
		if (m_InheritOwnerLinearVelocity && SpawnedActor->GetOwner())
		{
			InheritedVelocity = SpawnedActor->GetOwner()->GetVelocity();
		}

		if (!m_InitialVelocity.IsNearlyZero())
		{
			// Use the Projectile Movement Component if they have one setup since this is likely used for spawning projectiles.
			if (UProjectileMovementComponent* ProjectileComponent = SpawnedActor->FindComponentByClass<UProjectileMovementComponent>())
			{
				ProjectileComponent->SetVelocityInLocalSpace(m_InitialVelocity + InheritedVelocity);
			}
			else if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(SpawnedActor->GetRootComponent()))
			{
				PrimitiveComponent->AddImpulse(m_InitialVelocity + InheritedVelocity);
			}
		}

		if (m_AttachToOwnerSocket)
		{
			if (USkeletalMeshComponent* SkeletalComponent = SpawnedActor->GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (USceneComponent* SceneComponent = SpawnedActor->FindComponentByClass<USceneComponent>())
				{
					FAttachmentTransformRules AttachRules(m_AttachmentRule, false);
					SceneComponent->AttachToComponent(SkeletalComponent, AttachRules, m_SocketName);
				}
			}
		}

		if (m_DestroyAtEnd)
		{
			ScratchPad->SpawnedActor = SpawnedActor;
		}

		if (m_FireEvent)
		{
#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Calling OnSpawnedActorEvent with event name %s and Spawned Actor %s."), *m_Name.ToString(), *SpawnedActor->GetName()));
			}
#endif
			Context->GetAbility()->OnSpawnedActorEvent(Context.Get(), m_Name, SpawnedActor);
		}
	}

}

void UAblSpawnActorTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const
{
	Super::OnTaskEnd(Context, result);

	if (m_DestroyAtEnd)
	{
		UAblSpawnActorTaskScratchPad* ScratchPad = Cast<UAblSpawnActorTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);

		if (ScratchPad->SpawnedActor.IsValid())
		{
#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Destroying Spawned Actor %s."), *ScratchPad->SpawnedActor->GetName()));
			}
#endif
			UWorld* SpawnedWorld = ScratchPad->SpawnedActor->GetWorld();
			SpawnedWorld->DestroyActor(ScratchPad->SpawnedActor.Get());
		}
	}
}

UAblAbilityTaskScratchPad* UAblSpawnActorTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return m_DestroyAtEnd ? NewObject<UAblSpawnActorTaskScratchPad>(Context.Get()) : nullptr;
}

TStatId UAblSpawnActorTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblSpawnActorTask, STATGROUP_Able);
}

#if WITH_EDITOR

FText UAblSpawnActorTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblSpawnActorTaskFormat", "{0}: {1}");
	FString ActorName = TEXT("<null>");
	if (*m_ActorClass)
	{
		if (AActor* Actor = Cast<AActor>(m_ActorClass->GetDefaultObject()))
		{
			ActorName = Actor->GetName();
		}
	}

	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(ActorName));
}

#endif

#undef LOCTEXT_NAMESPACE