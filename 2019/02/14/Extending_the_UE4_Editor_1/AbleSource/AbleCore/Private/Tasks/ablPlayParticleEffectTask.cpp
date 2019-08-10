// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablPlayParticleEffectTask.h"

#include "ablAbility.h"
#include "AbleCorePrivate.h"
#include "Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Public/ParticleEmitterInstances.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblPlayParticleEffectTaskScratchPad::UAblPlayParticleEffectTaskScratchPad()
{

}

UAblPlayParticleEffectTaskScratchPad::~UAblPlayParticleEffectTaskScratchPad()
{

}

UAblPlayParticleEffectTask::UAblPlayParticleEffectTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_EffectTemplate(nullptr),
	m_SocketName(NAME_None),
	m_AttachToSocket(false),
	m_UseSocketRotation(false),
	m_LocationOffset(),
	m_RotationOffset(),
	m_Scale(1.0f),
	m_DestroyAtEnd(false)
{

}

UAblPlayParticleEffectTask::~UAblPlayParticleEffectTask()
{

}

void UAblPlayParticleEffectTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	if (!m_EffectTemplate)
	{
		UE_LOG(LogAble, Warning, TEXT("No Particle System set for PlayParticleEffectTask in Ability [%s]"), *Context->GetAbility()->GetDisplayName());
		return;
	}

	FTransform OffsetTransform(m_RotationOffset, m_LocationOffset);
	FTransform SpawnTransform = OffsetTransform;
	
	TWeakObjectPtr<UParticleSystemComponent> SpawnedEffect = nullptr;

	UAblPlayParticleEffectTaskScratchPad* ScratchPad = nullptr;
	if (m_DestroyAtEnd)
	{
		ScratchPad = Cast<UAblPlayParticleEffectTaskScratchPad>(Context->GetScratchPadForTask(this));
	}

	TArray<TWeakObjectPtr<AActor>> TargetArray;
	GetActorsForTask(Context, TargetArray);

	for (TWeakObjectPtr<AActor>& Target : TargetArray)
	{
		if (m_SocketName.IsNone())
		{
			if (Target.IsValid())
			{
				SpawnTransform = Target->GetActorTransform() * OffsetTransform;
				SpawnTransform.SetScale3D(FVector(m_Scale));
			}

#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Spawning Emitter %s with Transform %s"), *m_EffectTemplate->GetName(), *SpawnTransform.ToString()));
			}
#endif

			SpawnedEffect = UGameplayStatics::SpawnEmitterAtLocation(Target->GetWorld(), m_EffectTemplate, SpawnTransform);
		}
		else if (Target.IsValid())
		{
			if (m_AttachToSocket)
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Spawning Attached Emitter %s using Socket %s"), *m_EffectTemplate->GetName(), *m_SocketName.ToString()));
				}
#endif
				SpawnedEffect = UGameplayStatics::SpawnEmitterAttached(m_EffectTemplate, Target->FindComponentByClass<USceneComponent>(), m_SocketName, m_LocationOffset, m_RotationOffset);
			}
			else // Just use the socket as an initial transform.
			{
				if (USkeletalMeshComponent* MeshComponent = Target->FindComponentByClass<USkeletalMeshComponent>())
				{
					if (m_UseSocketRotation)
					{
						SpawnTransform = MeshComponent->GetSocketTransform(m_SocketName, RTS_Component) * OffsetTransform;
					}
					else
					{
						SpawnTransform = FTransform(MeshComponent->GetSocketLocation(m_SocketName)) * OffsetTransform;
					}

					SpawnTransform.SetScale3D(FVector(m_Scale));

#if !(UE_BUILD_SHIPPING)
					if (IsVerbose())
					{
						PrintVerbose(FString::Printf(TEXT("Spawning Emitter %s with Transform %s"), *m_EffectTemplate->GetName(), *SpawnTransform.ToString()));
					}
#endif

					SpawnedEffect = UGameplayStatics::SpawnEmitterAtLocation(Target->GetWorld(), m_EffectTemplate, SpawnTransform);
				}
			}
		}

		if (m_DestroyAtEnd && SpawnedEffect.IsValid() && ScratchPad)
		{
			ScratchPad->SpawnedEffects.Add(SpawnedEffect);
		}

		// Set our Parameters.
		for (UAblParticleEffectParam* Parameter : m_Parameters)
		{
			if (Parameter->IsA<UAblParticleEffectParamContextActor>())
			{
				UAblParticleEffectParamContextActor* ContextActorParam = Cast<UAblParticleEffectParamContextActor>(Parameter);
				if (AActor* FoundActor = GetSingleActorFromTargetType(Context, ContextActorParam->GetContextActorType()))
				{
					SpawnedEffect->SetActorParameter(ContextActorParam->GetParameterName(), FoundActor);
				}
			}
			else if (Parameter->IsA<UAblParticleEffectParamLocation>())
			{
				UAblParticleEffectParamLocation* LocationParam = Cast<UAblParticleEffectParamLocation>(Parameter);
				FTransform outTransform;
				LocationParam->GetLocation().GetTransform(*Context.Get(), outTransform);
				SpawnedEffect->SetVectorParameter(LocationParam->GetParameterName(), outTransform.GetTranslation());
			}
		}
	}

}

void UAblPlayParticleEffectTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult Result) const
{
	Super::OnTaskEnd(Context, Result);

	if (m_DestroyAtEnd)
	{
		UAblPlayParticleEffectTaskScratchPad* ScratchPad = Cast<UAblPlayParticleEffectTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);

		for (TWeakObjectPtr<UParticleSystemComponent> SpawnedEffect : ScratchPad->SpawnedEffects)
		{
			if (SpawnedEffect.IsValid())
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Destroying Emitter %s"), *SpawnedEffect->GetName()));
				}
#endif
				SpawnedEffect->bAutoDestroy = true;
				SpawnedEffect->DeactivateSystem();
			}
		}
	}
}

UAblAbilityTaskScratchPad* UAblPlayParticleEffectTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return m_DestroyAtEnd ? NewObject<UAblPlayParticleEffectTaskScratchPad>(Context.Get()) : nullptr;
}

TStatId UAblPlayParticleEffectTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblPlayParticleEffectTask, STATGROUP_Able);
}

#if WITH_EDITOR

FText UAblPlayParticleEffectTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblPlayParticleEffectTaskFormat", "{0}: {1}");
	FString ParticleName = TEXT("<null>");
	if (m_EffectTemplate)
	{
		ParticleName = m_EffectTemplate->GetName();
	}
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(ParticleName));
}

#endif

#undef LOCTEXT_NAMESPACE

