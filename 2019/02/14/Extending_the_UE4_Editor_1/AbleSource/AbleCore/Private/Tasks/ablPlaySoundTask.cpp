// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablPlaySoundTask.h"

#include "ablAbility.h"
#include "AbleCorePrivate.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblPlaySoundTaskScratchPad::UAblPlaySoundTaskScratchPad()
{

}

UAblPlaySoundTaskScratchPad::~UAblPlaySoundTaskScratchPad()
{

}

UAblPlaySoundTask::UAblPlaySoundTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Sound(nullptr),
	m_2DSound(false),
	m_LocationOffset(),
	m_SocketName(NAME_None),
	m_AttachToSocket(false),
	m_DestroyOnEnd(false),
	m_DestroyFadeOutDuration(0.25f)
{

}

UAblPlaySoundTask::~UAblPlaySoundTask()
{

}

void UAblPlaySoundTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	if (!m_Sound)
	{
		UE_LOG(LogAble, Warning, TEXT("No Sound set for PlaySoundTask in Ability [%s]"), *Context->GetAbility()->GetDisplayName());
		return;
	}

	FTransform OffsetTransform(m_LocationOffset);
	FTransform SpawnTransform = OffsetTransform;
	
	if (m_2DSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), m_Sound);
		return; // We can early out, no reason to play a 2D sound multiple times.
	}

	TWeakObjectPtr<UAudioComponent> AttachedSound = nullptr;
	
	TArray<TWeakObjectPtr<AActor>> TargetArray;
	GetActorsForTask(Context, TargetArray);
	
	UAblPlaySoundTaskScratchPad* ScratchPad = nullptr;
	if (m_DestroyOnEnd)
	{
		ScratchPad = Cast<UAblPlaySoundTaskScratchPad>(Context->GetScratchPadForTask(this));
	}

	for (TWeakObjectPtr<AActor>& Target : TargetArray)
	{
		if (Target.IsValid())
		{
			if (m_SocketName.IsNone())
			{
				SpawnTransform = Target->GetActorTransform() * OffsetTransform;

#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Spawning Sound %s with Transform %s"), *m_Sound->GetName(), *SpawnTransform.ToString()));
				}
#endif

				AttachedSound = UGameplayStatics::SpawnSoundAtLocation(Target->GetWorld(), m_Sound, SpawnTransform.GetTranslation());
			}
			else
			{
				if (!m_AttachToSocket)
				{
					if (USkeletalMeshComponent* SkeletalComponent = Target->FindComponentByClass<USkeletalMeshComponent>())
					{
						SpawnTransform = SkeletalComponent->GetSocketTransform(m_SocketName) * OffsetTransform;

#if !(UE_BUILD_SHIPPING)
						if (IsVerbose())
						{
							PrintVerbose(FString::Printf(TEXT("Spawning Sound %s with Transform %s"), *m_Sound->GetName(), *SpawnTransform.ToString()));
						}
#endif

						AttachedSound = UGameplayStatics::SpawnSoundAtLocation(Target->GetWorld(), m_Sound, SpawnTransform.GetTranslation());
					}
				}
				else
				{
#if !(UE_BUILD_SHIPPING)
					if (IsVerbose())
					{
						PrintVerbose(FString::Printf(TEXT("Spawning Attached Sound %s with using Socket %s"), *m_Sound->GetName(), *m_SocketName.ToString()));
					}
#endif
					AttachedSound = UGameplayStatics::SpawnSoundAttached(m_Sound, Target->FindComponentByClass<USceneComponent>(), m_SocketName, m_LocationOffset, EAttachLocation::KeepRelativeOffset);
				}
			}

			if (AttachedSound.IsValid() && ScratchPad)
			{
				ScratchPad->AttachedSounds.Add(AttachedSound);
			}

			AttachedSound = nullptr;
		}
	}


}

void UAblPlaySoundTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult Result) const
{
	Super::OnTaskEnd(Context, Result);

	if (m_DestroyOnEnd)
	{
		if (UAblPlaySoundTaskScratchPad* ScratchPad = Cast<UAblPlaySoundTaskScratchPad>(Context->GetScratchPadForTask(this)))
		{
			for (TWeakObjectPtr<UAudioComponent>& AudioComponent : ScratchPad->AttachedSounds)
			{
				if (AudioComponent.IsValid())
				{
#if !(UE_BUILD_SHIPPING)
					if (IsVerbose())
					{
						PrintVerbose(FString::Printf(TEXT("Destroying Sound %s"), *m_Sound->GetName()));
					}
#endif
					AudioComponent->bAutoDestroy = true;
					AudioComponent->FadeOut(m_DestroyFadeOutDuration, 0.0f);
				}
			}
		}
	}
}

UAblAbilityTaskScratchPad* UAblPlaySoundTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return m_DestroyOnEnd ? NewObject<UAblPlaySoundTaskScratchPad>(Context.Get()) : nullptr;
}

TStatId UAblPlaySoundTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblPlaySoundTask, STATGROUP_Able);
}

#if WITH_EDITOR

FText UAblPlaySoundTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblPlaySoundTaskFormat", "{0}: {1}");
	FString SoundName = TEXT("<null>");
	if (m_Sound)
	{
		SoundName = m_Sound->GetName();
	}
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(SoundName));
}

#endif

#undef LOCTEXT_NAMESPACE

