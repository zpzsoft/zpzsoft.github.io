// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "AlphaBlend.h"
#include "Components/SkeletalMeshComponent.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/ObjectMacros.h"

#include "ablPlayAnimationTask.generated.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

class UAnimationAsset;
class UAblAbilityComponent;
class UAblAbilityContext;

/* Helper Struct for Blend Times. */
USTRUCT()
struct ABLECORE_API FAblBlendTimes
{
public:
	GENERATED_USTRUCT_BODY();
	
	UPROPERTY(EditInstanceOnly, Category = "Blend", meta = (DisplayName = "Blend In"))
	float m_BlendIn;

	// Blend out time (in seconds).
	UPROPERTY(EditInstanceOnly, Category = "Blend", meta = (DisplayName = "Blend Out"))
	float m_BlendOut;

	FAblBlendTimes()
		: m_BlendIn(0.25f),
		m_BlendOut(0.25f)
	{}
};

/* Scratchpad for our Task. */
UCLASS(Transient)
class UAblPlayAnimationTaskScratchPad : public UAblAbilityTaskScratchPad
{
	GENERATED_BODY()
public:
	UAblPlayAnimationTaskScratchPad();
	virtual ~UAblPlayAnimationTaskScratchPad();

	/* The Ability Components of all the actors we affected. */
	UPROPERTY(transient)
	TArray<TWeakObjectPtr<UAblAbilityComponent>> AbilityComponents;

	/* The Skeletal Mesh Components of all the actors we affected (Single Node only). */
	UPROPERTY(transient)
	TArray<TWeakObjectPtr<USkeletalMeshComponent>> SingleNodeSkeletalComponents;
};

UENUM()
enum EAblPlayAnimationTaskAnimMode
{
	SingleNode UMETA(DisplayName="Single Node"),
	AbilityAnimationNode UMETA(DisplayName = "Ability Animation Node"),
	DynamicMontage UMETA(DisplayName = "Dynamic Montage")
};

UCLASS()
class ABLECORE_API UAblPlayAnimationTask : public UAblAbilityTask
{
	GENERATED_BODY()

public:
	UAblPlayAnimationTask(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblPlayAnimationTask();

	/* Start our Task. */
	virtual void OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const override;
	
	/* End our Task. */
	virtual void OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const override;

	/* Returns the End time of our Task. */
	virtual float GetEndTime() const override;

	/* Returns which realm this Task belongs to. */
	virtual EAblAbilityTaskRealm GetTaskRealm() const override { return m_PlayOnServer ? EAblAbilityTaskRealm::ClientAndServer : EAblAbilityTaskRealm::Client; }

	/* Creates the Scratchpad for our Task. */
	virtual UAblAbilityTaskScratchPad* CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const;

	/* Returns the Profiler Stat ID for our Task. */
	virtual TStatId GetStatId() const override;

	virtual void OnAbilityTimeSet(const TWeakObjectPtr<const UAblAbilityContext>& Context) override;

#if WITH_EDITOR
	/* Returns the category of our Task. */
	virtual FText GetTaskCategory() const override { return LOCTEXT("AblPlayAnimationTaskCategory", "Animation"); }
	
	/* Returns the name of our Task. */
	virtual FText GetTaskName() const override { return LOCTEXT("AblPlayAnimationTask", "Play Animation"); }

	/* Returns the dynamic, descriptive name of our Task. */
	virtual FText GetDescriptiveTaskName() const override;
	
	/* Returns the description of our Task. */
	virtual FText GetTaskDescription() const override { return LOCTEXT("AblPlayAnimationTaskDesc", "Plays an Animation asset (currently only Animation Montage and Animation Segments are supported)."); }
	
	/* Returns the color of our Task. */
	virtual FLinearColor GetTaskColor() const override { return FLinearColor(0.0f, 1.0f, 1.0f); } // Light Blue
	
	/* Returns the estimated runtime cost of our Task. */
	virtual float GetEstimatedTaskCost() const override { return ABLTASK_EST_SYNC; }

	/* Returns how to display the End time of our Task. */
	virtual EVisibility ShowEndTime() const override { return EVisibility::Hidden; } // Hidden = Read only (it's still using space in the layout, so might as well display it).
#endif
	/* Returns the Animation Asset. */
	FORCEINLINE const UAnimationAsset* GetAnimationAsset() const { return m_AnimationAsset; }
	
	/* Sets the Animation Asset. */
	void SetAnimationAsset(const UAnimationAsset* Animation);

	/* Returns the Animation Mode. */
	FORCEINLINE EAblPlayAnimationTaskAnimMode GetAnimationMode() const { return m_AnimationMode.GetValue(); }
	
	/* Returns the State Machine Name. */
	FORCEINLINE const FName& GetStateMachineName() const { return m_StateMachineName; }
	
	/* Returns the Ability State Name. */
	FORCEINLINE const FName& GetAbilityStateName() const { return m_AbilityStateName; }

protected:
	/* Helper method to clean up code a bit. This method does the actual PlayAnimation/Montage_Play/etc call.*/
	void PlayAnimation(AActor& TargetActor, UAblPlayAnimationTaskScratchPad& ScratchPad, USkeletalMeshComponent& SkeletalMeshComponent, float PlayRate) const;

	/* Helper method to find the AbilityAnimGraph Node, if it exists. */
	struct FAnimNode_AbilityAnimPlayer* GetAbilityAnimGraphNode(USkeletalMeshComponent* MeshComponent) const;

	// The Animation to play.
	UPROPERTY(EditAnywhere, Category="Animation", meta=(DisplayName="Animation", AllowedClasses="AnimMontage, AnimSequence"))
	const UAnimationAsset* m_AnimationAsset;

	/* What mode to use for this task. 
	*  Single Node - Plays the Animation as a Single Node Animation outside of the Animation Blueprint (if there is one).
	*  Ability Animation Node - Plays the Animation using the Ability Animation Node within an Animation State Machine.
	*  Dynamic Montage - Plays the Animation as a Dynamic Montage.
	*/
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Animation Mode", EditCondition = "m_AnimationAsset!=nullptr"))
	TEnumAsByte<EAblPlayAnimationTaskAnimMode> m_AnimationMode;

	/* The name of the State Machine we should look for our Ability State in.*/
	UPROPERTY(EditAnywhere, Category = "Ability Node", meta = (DisplayName = "State Machine Name", EditCondition = "m_AnimationMode == EAblPlayAnimationTaskAnimMode::AbilityAnimationNode"))
	FName m_StateMachineName;

	/* The name of the State that contains the Ability Animation Player Node*/
	UPROPERTY(EditAnywhere, Category = "Ability Node", meta = (DisplayName = "Ability State Name", EditCondition = "m_AnimationMode == EAblPlayAnimationTaskAnimMode::AbilityAnimationNode"))
	FName m_AbilityStateName;

	/* If the node is already playing an animation, this is the blend used transition to this animation.*/
	UPROPERTY(EditAnywhere, Category = "Ability Node", meta = (DisplayName = "Blend In", EditCondition = "m_AnimationMode == EAblPlayAnimationTaskAnimMode::AbilityAnimationNode"))
	FAlphaBlend m_BlendIn;

	// Whether or not to loop the animation.
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Loop", EditCondition="m_AnimationAsset!=nullptr"))
	bool m_Loop;

	// Blend times to use when playing the animation as a dynamic montage.
	UPROPERTY(EditAnywhere, Category = "Dynamic Montage", meta = (DisplayName = "Play Blend", EditCondition = "m_AnimationMode == EAblPlayAnimationTaskAnimMode::DynamicMontage"))
	FAblBlendTimes m_DynamicMontageBlend;

	// Animation Play Rate
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Play Rate", EditCondition = "m_AnimationAsset!=nullptr"))
	float m_PlayRate;

	// If true, we scale our Animation Play Rate by what our Ability Play Rate is. So, if your Ability Play Rate is 2.0, the animation play rate is multiplied by that same value.
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Scale With Ability Play Rate", EditCondition = "m_AnimationAsset!=nullptr"))
	bool m_ScaleWithAbilityPlayRate;

	// If true, stop the current playing animation mid blend if the owning Ability is interrupted. Only applies when Animation Mode is set to Ability Animation Node.
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Stop on Interrupt", EditCondition = "m_AnimationAsset!=nullptr"))
	bool m_StopAnimationOnInterrupt;

	/* If true, in a networked game, the animation will be played on the server. 
	*  You should only use this if you have collision queries that rely on bone positions
	*  or animation velocities.
	*/
	UPROPERTY(EditAnywhere, Category = "Network", meta=(DisplayName="Play On Server", EditCondition = "m_AnimationAsset!=nullptr"))
	bool m_PlayOnServer;
};

#undef LOCTEXT_NAMESPACE
