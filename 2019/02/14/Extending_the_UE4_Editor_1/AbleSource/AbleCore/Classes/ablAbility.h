// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "ablAbilityContext.h"

#include "Channeling/ablChannelingBase.h"
#include "GameplayTagContainer.h"
#include "Targeting/ablTargetingBase.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

#include "ablAbility.generated.h"

#define LOCTEXT_NAMESPACE "AblAbility"

class UAblAbilityTask;

UENUM(BlueprintType)
enum class EAblAbilityStartResult : uint8
{
	InvalidTarget UMETA(DisplayName = "Invalid Target"),
	FailedCustomCheck UMETA(DisplayName = "Failed Custom Check"),
	CooldownNotExpired UMETA(DisplayName = "Cooldown Not Expired"),
	CannotInterruptCurrentAbility UMETA(DisplayName = "Cannot Interrupt Current Ability"),
	NotAllowedAsPassive UMETA(DisplayName = "Not Allowed as Passive"),
	PassiveMaxStacksReached UMETA(DisplayName = "Passive Max Stacks Reached"),
	InternalSystemsError UMETA(DisplayName = "Internal Systems Error"),
	AsyncProcessing UMETA(DisplayName = "Async Processing"), // Used when an Async query has been queued up
	ForwardedToServer UMETA(DisplayName = "Forwarded to Server"),
	InvalidParameter UMETA(DisplayName = "Invalid Parameter"),
	Success UMETA(DisplayName = "Success")
};

UENUM()
enum class EAblAbilityPassiveBehavior : uint8
{
	CannotBePassive UMETA(DisplayName="Cannot be Passive"),
	RefreshDuration UMETA(DisplayName="Refresh Duration"),
	IncreaseStackCount UMETA(DisplayName="Increase Stack Count"),
	IncreaseAndRefresh UMETA(DisplayName="Increase Stack and Refresh Duration")
};

UCLASS(Blueprintable, hidecategories=(Internal, Thumbnail), meta = (DisplayThumbnail = "true"))
class ABLECORE_API UAblAbility : public UObject
{
	GENERATED_BODY()

public:
	UAblAbility(const FObjectInitializer& ObjectInitializer);

	// UObject Overrides.
	void PostInitProperties() override;
	void PostLoad() override;

	/* Check all prerequisites using the ability context to see if this ability can be executed.
	 * This will also check any targeting logic and populate the Context with that information.
	 * @param FAbilityContext - The context for this ability. If it has targeting, this call will populate the context with those targets.
	 * 
	 * @return EAblAbilityStartResult - Result code.
	 */
	EAblAbilityStartResult CanAbilityExecute(UAblAbilityContext& Context) const;

	/* Returns the name of the Ability in a print friendly format (without class identifiers). */
	UFUNCTION(BlueprintPure, Category="Able|Ability")
	FString GetDisplayName() const;

	/* Returns the Channeling Conditions. */
	FORCEINLINE const TArray<UAblChannelingBase*>& GetChannelConditions() const { return m_ChannelConditions; }
	
	/* Returns the Ability Tasks. */
	FORCEINLINE const TArray<UAblAbilityTask*>& GetTasks() const { return m_Tasks; }
	
	/* Returns the Ability Name Hash. */
	FORCEINLINE uint32 GetAbilityNameHash() const { return m_AbilityNameHash; }
	
	/* Returns the realm this Ability belongs to. */
	FORCEINLINE EAblAbilityTaskRealm GetAbilityRealm() const { return m_AbilityRealm.GetValue(); }

	/* Returns the result used when Channeling Fails. */
	FORCEINLINE EAblAbilityTaskResult GetChannelFailureResult() const { return m_FailedChannelResult.GetValue(); }

	/* Returns any Task dependencies. */
	FORCEINLINE const TArray<const UAblAbilityTask*>& GetAllTaskDependencies() const { return m_AllDependentTasks; }

	/* Returns the Length, in seconds, of the Ability. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE float GetLength() const { return m_Length; }

	/* Returns the Play rate as set on the Ability. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE float GetBasePlayRate() const { return m_PlayRate; }
	
	/* Returns true if the Ability requires a Target. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE bool RequiresTarget() const { return m_RequiresTarget; }

	/* Returns the estimated range of the Ability. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE float GetRange() const { return m_RequiresTarget ? m_Targeting->GetRange() : 0.0f; }
	
	/* Returns the Cooldown, in seconds, of the Ability. DEPRECATED: Use GetBaseCooldown if you want this value now. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability", meta=(DisplayName="GetCooldown_Deprecated", DeprecatedFunction))
	FORCEINLINE float GetCooldown() const { return m_Cooldown; }

	/* Returns the Base Cooldown, in seconds, of the Ability. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE float GetBaseCooldown() const { return m_Cooldown; }

	/* Returns the Base Stacks value of the Ability. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE int32 GetBaseMaxStacks() const { return m_MaxStacks; }

	/* Returns the Ability Tag Container. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE FGameplayTagContainer GetAbilityTagContainer() const { return m_TagContainer; }
	
	/* Returns true if the Ability is set to loop. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE bool IsLooping() const { return m_Loop; }
	
	/* Returns the Max Loop Iterations. */
	FORCEINLINE uint32 GetLoopMaxIterations() const { return m_LoopMaxIterations; }

	/* Returns the Max Loop Iterations. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability", DisplayName="GetLoopMaxIterations")
	FORCEINLINE int32 GetLoopMaxIterationsBP() const { return (int32)m_LoopMaxIterations; }

	/* Returns the Loop range (in seconds) of this Ability. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FVector2D GetLoopRange() const { return FVector2D(m_LoopStart, m_LoopEnd); }

	/* Returns true if this Ability is a passive Ability. */
	UFUNCTION(BlueprintPure, Category="Able|Ability")
	FORCEINLINE bool IsPassive() const { return m_IsPassive; }

	/* Returns true if the duration should be reset any time a new stack is applied to the Ability. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE bool RefreshDurationOnNewStack() const { return m_RefreshDurationOnNewStack; }

	/* Returns true if the duration is always refreshed, regardless if a new stack is added or fails. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE bool AlwaysRefreshDuration() const { return m_AlwaysRefreshDuration; }

	/* Returns true if, when refreshing duration, only the Loop range should be refreshed. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	FORCEINLINE bool RefreshLoopTimeOnly() const { return m_OnlyRefreshLoopTime; }

	/* Returns true if the Ability is channeled. */
	UFUNCTION(BlueprintPure, Category="Able|Ability")
	FORCEINLINE bool IsChanneled() const { return m_IsChanneled; }

	/* Returns true if the Ability must pass all channel conditions to continue channeling. */
	UFUNCTION(BlueprintPure, Category="Able|Ability")
	FORCEINLINE bool MustPassAllChannelConditions() const { return m_MustPassAllChannelConditions; }
	
	/* Called when THIS Ability is attempting to interrupt another ability.
	* @return - Whether to allow the this ability to interrupt or not.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Can Interrupt Ability")
	bool CanInterruptAbility(const UAblAbilityContext* Context, const UAblAbility* CurrentAbility) const;
	
	/* Called by the Custom Filter for each actor that is a candidate to be acted upon by this Ability.
	* @return - Whether to allow this Actor as a valid target or not.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Custom Filter Condition")
	bool CustomFilterCondition(const UAblAbilityContext* Context, const FName& EventName, AActor* Actor) const;

	/* Called when determining the cooldown for an Ability
	* If you have any special logic that increases/decreases the cooldown amount, you can do it here.
	* @return - The amount of time, in seconds, to use for the cooldown. By default, returns GetBaseCooldown.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Get Cooldown")
	float CalculateCooldown(const UAblAbilityContext* Context) const;

	/* Called when grabbing the Max stack count for this Ability (if it's a passive). 
	* If you have any special logic that increases/decreases the Stack amount, you can do it here.
	* @return - The maximum number of stacks to allow, must be greater than Zero.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Get Max Stacks")
	int32 GetMaxStacks(const UAblAbilityContext* Context) const;

	/* Called when checking if an Ability is valid to be started. You can use this event to execute
	 * any custom logic beyond the normal targeting/resource checks.
	 * @return - True if the ability should be allowed to start, false if not.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName="Can Ability Execute")
	bool CustomCanAbilityExecute(const UAblAbilityContext* Context) const;

	/* Called by the Custom Targeting check. Any actors added to the FoundTargets value will be copied into the Ability Context.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "Find Targets")
	void CustomTargetingFindTargets(const UAblAbilityContext* Context, TArray<AActor*>& FoundTargets) const;

	/* Called when checking if the looping segment of an Ability is valid to be started. 
	* @return - True if the loop should be allowed to start, false if not.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Can Loop Execute")
	bool CustomCanLoopExecute(const UAblAbilityContext* Context) const;

	/* Called by the Custom Channeling Condition. 
	* @return - True if the channeling should be allowed to continue, false if not.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Custom Channel Conditional")
	bool CheckCustomChannelConditional(const UAblAbilityContext* Context, const FName& EventName) const;

	/* Called when the Damage event needs to calculate a final damage value for the provided Actor. 
	*  NOTE: This method should always be thread safe, so only read data from actors - never write/change values of an actor. */
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Calculate Damage For Actor")
	float CalculateDamageForActor(const UAblAbilityContext* Context, const FName& EventName, float BaseDamage, AActor* Actor) const;

	/* Called by the Custom Branch Conditional. 
	* @return - True if the Branch Ability should be allowed to start, false if not.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Can Branch To")
	bool CustomCanBranchTo(const UAblAbilityContext* Context, const UAblAbility* BranchAbility) const;

	/* Called when the Ability has passed all prerequisites (if any) and has actually started execution. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName="On Ability Start")
	void OnAbilityStart(const UAblAbilityContext* Context) const;

	/* Called when Ability has successfully executed all Ability Tasks. This is NOT called if the Ability is interrupted/branched.
	 * Use OnAbilityInerrupted/OnAbilityBranch to catch those cases.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "On Ability End")
	void OnAbilityEnd(const UAblAbilityContext* Context) const;

	/* Called when an Ability is interrupted. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "On Ability Interrupt")
	void OnAbilityInterrupt(const UAblAbilityContext* Context) const;

	/* Called when an Ability is being branched to another Ability. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "On Ability Branch")
	void OnAbilityBranch(const UAblAbilityContext* Context) const;

	/* Generic event called when a Task does a Collision query/Sweep. You can use EventName to identify which Task called this event. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "On Collision Event")
	void OnCollisionEvent(const UAblAbilityContext* Context, const FName& EventName, const TArray<struct FAblQueryResult>& HitEntities) const;

	/* Generic event called when a Task does a Raycast query. You can use EventName to identify which Task called this event. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "On Raycast Event")
	void OnRaycastEvent(const UAblAbilityContext* Context, const FName& EventName, const TArray<FHitResult>& HitResults) const;

	/* Generic event called when by the Custom event Task. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "On Custom Event")
	void OnCustomEvent(const UAblAbilityContext* Context, const FName& EventName) const;

	/* Generic event called by the Spawn Actor Task. You can use EventName to identify which Task called this event.*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Able|Ability", DisplayName = "On Spawned Actor")
	void OnSpawnedActorEvent(const UAblAbilityContext* Context, const FName& EventName, AActor* SpawnedActor) const;

	/* Called by the Branch Task if the dynamic branch ability option is enabled on the Task itself.
	* @return - The Ability to use as the Branching Ability.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "On Get Branch Ability")
	UAblAbility* OnGetBranchAbility(const UAblAbilityContext* Context, const FName& EventName) const;

	/* Called when determining the play rate for an Ability
	* If you have any special logic that increases/decreases the play rate amount, you can do it here.
	* @return - The multiplier to apply to time while processing this Ability. By default, returns GetBasePlayRate.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "Able|Ability", DisplayName = "Get Play Rate")
	float GetPlayRate(const UAblAbilityContext* Context) const;

#if WITH_EDITOR
	/* Adds a Task to the Ability. */
	void AddTask(UAblAbilityTask& Task);
	
	/* Removes a Task from the Ability. */
	void RemoveTask(UAblAbilityTask& Task);
	
	/* Removes the Task at the provided index from the Ability. */
	void RemoveTaskAtIndex(int32 Index);

	/* Sorts all Tasks. */
	void SortTasks();

	/* Make sure all dependencies are valid, remove stale dependencies. */
	void ValidateDependencies();

	/* This is only used in the case where we inherit from an Ability that already contains task. */
	void MarkTasksAsPublic(); 

	/* Sets the Length of the Ability. */
	FORCEINLINE void SetLength(float Length) { m_Length = Length; }

	// UObject override. 
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#if WITH_EDITORONLY_DATA
	/* If the user has manually chosen a thumbnail image, use that. */
	UPROPERTY()
	class UTexture2D* ThumbnailImage;
	
	/** Information for thumbnail rendering */
	UPROPERTY(VisibleAnywhere, Instanced, Category = Thumbnail)
	class UThumbnailInfo* ThumbnailInfo;

	/* Event used to catch any of our Tasks modifying their properties. */
	void OnReferencedTaskPropertyModified(UAblAbilityTask& Task, struct FPropertyChangedEvent& PropertyChangedEvent);
#endif

private:
	/* Helper method to build our Task dependency list. */
	void BuildDependencyList();

	/* The Length, in seconds, this ability takes to successfully execute. Tasks that end after this value (if any)
	 * will simply be ended early. This could cause unwanted behavior, so try to keep all Task times within the overall 
	 * time of the ability. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (DisplayName = "Length", ClampMin = 0.0))
	float m_Length;

	/* The time before this ability can be used again. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (DisplayName = "Cooldown", ClampMin = 0.0))
	float m_Cooldown;

	/* The rate at which we play the Ability. 1.0 is normal, 2.0 is twice as fast, 0.5 is half as fast, etc. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (DisplayName = "Play Rate", ClampMin = 0.0))
	float m_PlayRate;

	/* Whether this ability is eligible to be a passive or not. Passives cannot be executed as Actives and vice-versa.*/
	UPROPERTY(EditDefaultsOnly, Category = "Passive", meta = (DisplayName = "Is Passive"))
	bool m_IsPassive;

	/* The Maximum number of stacks of this ability. */
	UPROPERTY(EditDefaultsOnly, Category = "Passive", meta = (DisplayName = "Max Stacks", ClampMin = 1, EditCondition=m_IsPassive))
	int32 m_MaxStacks;

	/* If true, we will only set the time for this ability if a new stack is successfully applied. */
	UPROPERTY(EditDefaultsOnly, Category = "Passive", meta = (DisplayName = "Refresh Duration On New Stack", ClampMin = 1, EditCondition = m_IsPassive))
	bool m_RefreshDurationOnNewStack;

	/* If true, the time will be reset for this ability, regardless of if a new stack was applied - or we were already at Max stacks. */
	UPROPERTY(EditDefaultsOnly, Category = "Passive", meta = (DisplayName = "Always Refresh Duration", ClampMin = 1, EditCondition = m_IsPassive))
	bool m_AlwaysRefreshDuration;

	/* If true, we will only reset the current loop time, instead of the ability time. You can use this if you have infinite looping abilities
	*  and you don't want certain tasks that happen before the loop to occur each time the passive is refreshed. */
	UPROPERTY(EditDefaultsOnly, Category = "Passive", meta = (DisplayName = "Only Refresh Loop Time", ClampMin = 1, EditCondition = m_IsPassive))
	bool m_OnlyRefreshLoopTime; 

	/* If true, and the loop passes the OnCustomCanLoopExecute event (if there is one), the ability will continue looping
	   the time segment specified until either the OnCustomCanLoopExecute returns false or the Max Iterations count is hit. */
	UPROPERTY(EditDefaultsOnly, Category = "Loop", meta = (DisplayName = "Is Looping"))
	bool m_Loop;

	/* The start time of the Looping segment.*/
	UPROPERTY(EditDefaultsOnly, Category = "Loop", meta = (DisplayName = "Loop Start", ClampMin = 0.0, EditCondition=m_Loop))
	float m_LoopStart;

	/* The end time of the Looping segment. */
	UPROPERTY(EditDefaultsOnly, Category = "Loop", meta = (DisplayName = "Loop End", ClampMin = 0.0, EditCondition = m_Loop))
	float m_LoopEnd;

	/* The number of iterations to perform the loop. 0 = Infinite*/
	UPROPERTY(EditDefaultsOnly, Category = "Loop", meta = (DisplayName = "Max Iterations", ClampMin = 0, EditCondition = m_Loop))
	uint32 m_LoopMaxIterations;

	/* If true, this ability will immediately fail to run if it cannot find a target using the targeting logic. */
	UPROPERTY(EditDefaultsOnly, Category = "Targeting", meta = (DisplayName = "Requires Target"))
	bool m_RequiresTarget;

	/* The targeting logic to use when attempting to find a target. */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Targeting", meta = (DisplayName = "Target Logic"))
	UAblTargetingBase* m_Targeting;

	/* Is this Ability a Channel Ability? Channeled Abilities have 1 or more Channel Conditionals that are checked each frame
	   if the condition passes, the Ability is allowed to continue execution. If it fails, the Ability is treated according to the provided Failed Channel Result.
	   Passive Abilities cannot be channeled.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Channeling", meta = (DisplayName = "Is Channeled", EditConditional = "!m_IsPassive"))
	bool m_IsChanneled;

	/* The conditions to check each frame to allow our channel to complete, or not. */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Channeling", meta = (DisplayName = "Channel Conditions", EditCondition = "m_IsChanneled"))
	TArray<UAblChannelingBase*> m_ChannelConditions;

	/* If true, all conditions must succeed to continue channeling the Ability. */
	UPROPERTY(EditDefaultsOnly, Category = "Channeling", meta = (DisplayName = "Must Pass All Conditions", EditCondition = "m_IsChanneled"))
	bool m_MustPassAllChannelConditions;

	/* What result to pass to the Ability when the channel condition fails. */
	UPROPERTY(EditDefaultsOnly, Category = "Channeling", meta = (DisplayName="Failed Channel Result", EditCondition = "m_IsChanneled"))
	TEnumAsByte<EAblAbilityTaskResult> m_FailedChannelResult;

	/* Tag container for any Tags you wish to assign to this ability. */
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (DisplayName = "Tag Container"))
	FGameplayTagContainer m_TagContainer;

	/* Our Tasks */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Internal")
	TArray<UAblAbilityTask*> m_Tasks;

	// Various run-time parameters.

	/* CRC Hash of our Ability Name. */
	UPROPERTY(Transient)
	uint32 m_AbilityNameHash;

	/* Realm of our Ability - calculated based on our Tasks, used in early-outs.*/
	UPROPERTY(Transient)
	TEnumAsByte<EAblAbilityTaskRealm> m_AbilityRealm;

	/* An Array of unique Tasks Dependencies. If one Task depends on another, the Task it depends on will be in this list. This is cached to save time during execution. */
	UPROPERTY(Transient)
	TArray<const UAblAbilityTask*> m_AllDependentTasks;
};

#undef LOCTEXT_NAMESPACE