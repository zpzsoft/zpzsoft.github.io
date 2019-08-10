// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Tasks/ablPlayParticleEffectParams.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablPlayParticleEffectTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

class UParticleSystem;
class UParticleSystemComponent;

/* Scratchpad for our Task. */
UCLASS(Transient)
class UAblPlayParticleEffectTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblPlayParticleEffectTaskScratchPad();
	virtual ~UAblPlayParticleEffectTaskScratchPad();

	/* All the Particle effects we've spawned. */
	UPROPERTY(transient)
	TArray<TWeakObjectPtr<UParticleSystemComponent>> SpawnedEffects;
};

UCLASS()
class ABLECORE_API UAblPlayParticleEffectTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblPlayParticleEffectTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblPlayParticleEffectTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* End our Task. */
	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult Result) const override;

	/* Returns true if our Task is Async. */
	virtual bool IsAsyncFriendly() const override { return true; }
	
	/* Returns true if the Task only lasts a single frame. */
	virtual bool IsSingleFrame() const override { return !m_DestroyAtEnd; }
	
	/* Returns the realm of our Task. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return EAblAbilityTaskRealm::Client; }

	/* Creates the Scratchpad for our Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;

	/* Returns the Profiler Stat ID of our Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category of our Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblPlayParticleEffectTaskCategory", "Effects"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblPlayParticleEffectTask", "Play Particle Effect"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblPlayParticleEffectTaskDesc", "Plays a particle effect, can be attached to a bone."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(0.25f, 0.87f, 0.32f); } // Slightly less bright Green 
#endif

protected:
	/* Particle Effect to play. */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName="Effect Template"))
	UParticleSystem* m_EffectTemplate;

	/* The socket within our mesh component to attach to, or use as an initial transform we spawn the particle component */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName="Socket"))
	FName m_SocketName;

	/* If true, the particle effect will follow the transform of the socket. */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName = "Attach To Socket"))
	bool m_AttachToSocket;

	/* If true, the particle effect will follow the transform of the socket. */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName = "Use Socket Rotation"))
	bool m_UseSocketRotation;

	/* Offset from the socket / bone location */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName="Location Offset"))
	FVector m_LocationOffset;

	/* Offset from the socket / bone rotation */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName="Rotation Offset"))
	FRotator m_RotationOffset;

	/* Uniform scale applied to the particle effect. */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName = "Scale"))
	float m_Scale;

	/* Whether or not we destroy the effect at the end of the task. */
	UPROPERTY(EditAnywhere, Category = "Particle", meta = (DisplayName = "Destroy on End"))
	bool m_DestroyAtEnd;

	/* Context Driven Parameters to set on the Particle instance.*/
	UPROPERTY(EditAnywhere, Instanced, Category = "Particle", meta = (DisplayName = "Instance Parameters"))
	TArray<UAblParticleEffectParam*> m_Parameters;
};

#undef LOCTEXT_NAMESPACE
