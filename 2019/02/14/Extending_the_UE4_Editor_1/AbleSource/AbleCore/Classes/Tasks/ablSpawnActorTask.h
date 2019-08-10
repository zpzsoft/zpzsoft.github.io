// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Targeting/ablTargetingBase.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablSpawnActorTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Scratchpad for this task. */
UCLASS(Transient)
class UAblSpawnActorTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblSpawnActorTaskScratchPad();
	virtual ~UAblSpawnActorTaskScratchPad();

	/* The Actor we've spawned. */
	UPROPERTY(transient)
	TWeakObjectPtr<AActor> SpawnedActor;
};

UCLASS()
class ABLECORE_API UAblSpawnActorTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblSpawnActorTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblSpawnActorTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* End our Task. */
	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const override;

	/* Returns true if our Task only lasts a single frame. */
	virtual bool IsSingleFrame() const override { return !m_DestroyAtEnd; }

	/* Returns the realm our Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return EAblAbilityTaskRealm::Server; }

	/* Creates the Scratchpad for this Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;

	/* Returns the Profiler Stat ID for this Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category of our Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblSpawnActorTaskCategory", "Spawn"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblSpawnActorTask", "Spawn Actor"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblSpawnActorTaskDesc", "Spawns an actor at the given location."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(0.0, 0.0, 0.4f); } // Dark Blue
	
	/* Returns the estimated runtime cost of our Task. */
	virtual float GetEstimatedTaskCost() const override { return UAblAbilityTask::GetEstimatedTaskCost() + ABLTASK_EST_SPAWN_ACTOR; }
#endif
protected:
	/* The class of the actor we want to dynamically spawn. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Actor Class"))
	TSubclassOf<AActor> m_ActorClass;

	/* The socket within our mesh component to attach the actor to, or use as an initial transform when we spawn the Actor */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Location"))
	FAblAbilityTargetTypeLocation m_SpawnLocation;

	/* The initial linear velocity for the spawned object. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Initial Velocity"))
	FVector m_InitialVelocity;

	/* If true, Set the owner of the new actor. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Set Owner"))
	bool m_SetOwner;

	/* The parent of the Actor. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Owner", EditCondition = "m_SetOwner"))
	TEnumAsByte<EAblAbilityTargetType> m_OwnerTargetType;

	/* If true, the object will be attached to the socket. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Attach To Owner Socket", EditCondition = "m_SetOwner"))
	bool m_AttachToOwnerSocket;

	/* The Attachment Rule to use if attached to the owner socket. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Attach To Owner Socket", EditCondition = "m_AttachToOwnerSocket"))
	EAttachmentRule m_AttachmentRule;

	/* The socket within our mesh component to attach the actor to, or use as an initial transform when we spawn the Actor */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Socket", EditCondition = "m_SetOwner && m_AttachToOwnerSocket"))
	FName m_SocketName;

	/* If true, spawn with our parent's linear velocity. This is added before the initial velocity. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Inherit Velocity", EditCondition = "m_SetOwner"))
	bool m_InheritOwnerLinearVelocity;

	/* If true, marks the Actor as transient, so it won't be saved between game sessions. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Transient"))
	bool m_MarkAsTransient;

	/* Whether or not we destroy the actor at the end of the task. */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (DisplayName = "Destroy On End"))
	bool m_DestroyAtEnd;

	/* If true, we'll call the OnSpawnedActorEvent in the Ability Blueprint. */
	UPROPERTY(EditAnywhere, Category = "Spawn|Event", meta = (DisplayName = "Fire Event"))
	bool m_FireEvent;

	/* A String identifier you can use to identify this specific task in the Ability blueprint. */
	UPROPERTY(EditAnywhere, Category = "Spawn|Event", meta = (DisplayName = "Name", EditCondition = m_FireEvent))
	FName m_Name;

};

#undef LOCTEXT_NAMESPACE
