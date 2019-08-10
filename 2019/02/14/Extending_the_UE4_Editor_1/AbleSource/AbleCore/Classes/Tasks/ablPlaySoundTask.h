// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Components/AudioComponent.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablPlaySoundTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Scratchpad for our Task. */
UCLASS(Transient)
class UAblPlaySoundTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblPlaySoundTaskScratchPad();
	virtual ~UAblPlaySoundTaskScratchPad();

	/* All the sounds we created. */
	UPROPERTY(transient)
	TArray<TWeakObjectPtr<UAudioComponent>> AttachedSounds;
};

UCLASS()
class ABLECORE_API UAblPlaySoundTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblPlaySoundTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblPlaySoundTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* End our Task. */
	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult Result) const override;

	/* Returns if our Task is Async. */
	virtual bool IsAsyncFriendly() const { return true; }
	
	/* Returns true if our Task only lasts a single frame. */
	virtual bool IsSingleFrame() const override { return !m_DestroyOnEnd; }
	
	/* Returns the realm this Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return EAblAbilityTaskRealm::Client; }

	/* Creates the Scratchpad for this Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;
	
	/* Returns the Profiler Stat ID for our Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category for our Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblPlaySoundTaskCategory", "Audio"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblPlaySoundTask", "Play Sound"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblPlaySoundTaskDesc", "Plays a sound at the given location, can be attached to socket, or played as a 2D (rather than 3D) sound."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(1.0, 1.0, 0.0f); } // Yellow
#endif

protected:
	/* The Sound to play. */
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (DisplayName = "Sound"))
	USoundBase* m_Sound;

	/* Plays the Sound as a 2D sound */
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (DisplayName = "Play as 2D"))
	bool m_2DSound;

	/* Offset, in local space, to play the sound. */
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (DisplayName = "Location Offset"))
	FVector m_LocationOffset;

	/* Name of the socket to use as the initial transform, in addition to the Location Offset. */
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (DisplayName = "Socket"))
	FName m_SocketName;

	/* Attach the sound to a socket. */
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (DisplayName = "Attach to Socket"))
	bool m_AttachToSocket;

	/* Stop the sound when the task ends. */
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (DisplayName = "Destroy on End"))
	bool m_DestroyOnEnd;

	/* If the sound is being destroyed early, how long, in seconds, to fade out so we don't have any hard audio stops. */
	UPROPERTY(EditAnywhere, Category = "Audio", meta = (DisplayName = "Fade Out Duration"))
	float m_DestroyFadeOutDuration;
};

#undef LOCTEXT_NAMESPACE