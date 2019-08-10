// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "AlphaBlend.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablTurnToTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

/* Helper Struct */
USTRUCT()
struct FTurnToTaskEntry
{
	GENERATED_USTRUCT_BODY()
public:
	FTurnToTaskEntry() {};
	FTurnToTaskEntry(AActor* InActor, FRotator InTarget)
		: Actor(InActor),
		Target(InTarget)
	{ }
	TWeakObjectPtr<AActor> Actor;
	FRotator Target;
};

/* Scratchpad for our Task. */
UCLASS(Transient)
class UAblTurnToTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblTurnToTaskScratchPad();
	virtual ~UAblTurnToTaskScratchPad();

	/* Any turns in progress. */
	UPROPERTY(transient)
	TArray<FTurnToTaskEntry> InProgressTurn;

	/* Blend to use for turns. */
	UPROPERTY(transient)
	FAlphaBlend TurningBlend;
};

UCLASS()
class ABLECORE_API UAblTurnToTask : public UAblAbilityTask
{
	GENERATED_BODY()
public:
	UAblTurnToTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblTurnToTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* On Task Tick. */
	virtual void OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const override;
	
	/* End our Task. */
	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const override;

	/* Returns true if our Task only lasts for a single frame. */
	virtual bool IsSingleFrame() const override { return m_Blend.IsComplete(); }
	
	/* Returns true if our Task needs its tick method called. */
	virtual bool NeedsTick() const override { return !IsSingleFrame(); }

	/* Returns the realm this Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return EAblAbilityTaskRealm::ClientAndServer; } // Client for Auth client, Server for AIs/Proxies.

	/* Creates the Scratchpad for this Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;

	/* Returns the Profiler Stat ID for this Task. */
	virtual TStatId GetStatId() const override;

#if WITH_EDITOR
	/* Returns the category of our Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblTurnToTaskCategory", "Movement"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblTurnToTask", "Turn To"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblTurnToTaskDesc", "Turns the task target towards the destination target over the duration of the task. If this is a single frame, then it will snap the character to the facing."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(1.0, 0.69, 0.4f); } // Peach

	// UObject Overrides
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	/* Helper method to get our Target rotation. */
	FRotator GetTargetRotation(const AActor* Source, const AActor* Destination) const;

	/* The Target we want to rotate towards. */
	UPROPERTY(EditAnywhere, Category = "Turn To", meta = (DisplayName = "Rotation Target"))
	TEnumAsByte<EAblAbilityTargetType> m_RotationTarget;

	/* If true, we'll continually try to turn towards this target for the duration of the task. If false, we only take the rotation value at the start of the task. */
	UPROPERTY(EditAnywhere, Category = "Turn To", meta = (DisplayName = "Track Target"))
	bool m_TrackTarget;

	/* An offset you can apply for the final rotation target. */
	UPROPERTY(EditAnywhere, Category = "Turn To", meta = (DisplayName = "Rotation Offset"))
	FRotator m_RotationOffset;

	/* If true, we will update the Yaw on actors this task affects. */
	UPROPERTY(EditAnywhere, Category = "Turn To", meta = (DisplayName = "Set Yaw"))
	bool m_SetYaw;

	/* If true, we will update the Pitch on actors this task affects. */
	UPROPERTY(EditAnywhere, Category = "Turn To", meta = (DisplayName = "Set Pitch"))
	bool m_SetPitch;

	/* The blend to use when we are turning. */
	UPROPERTY(EditAnywhere, Category = "Turn To", meta = (DisplayName = "Blend"))
	FAlphaBlend m_Blend;
};

#undef LOCTEXT_NAMESPACE
