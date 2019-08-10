// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "ablAbility.h"

#include "ablAbilityContext.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Tasks/IAblAbilityTask.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

#include "ablAbilityComponent.generated.h"

#define LOCTEXT_NAMESPACE "AbleCore"

class UAblAbility;
class UAblAbilityInstance;
class UAbleSettings;
class FAsyncAbilityCooldownUpdaterTask;
struct FAnimNode_AbilityAnimPlayer;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityStart, const UAblAbility& /*Ability*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAbilityEnd, const UAblAbility& /*Ability*/, EAblAbilityTaskResult /*Result*/);

/* Helper struct to keep track of Cooldowns. */
USTRUCT()
struct FAblAbilityCooldown
{
	GENERATED_USTRUCT_BODY()
public:
	FAblAbilityCooldown();
	FAblAbilityCooldown(const UAblAbility& InAbility, const UAblAbilityContext& Context);

	/* The Ability tied to this Cooldown. */
	const UAblAbility* GetAbility() const { return Ability.Get(); }

	/* Returns a value between 0 - 1.0 with how much time is left on the cooldown.*/
	float getTimeRatio() const { return FMath::Min(CurrentTime / CooldownTime, 1.0f);	}
	
	/* Returns if the Cooldown is complete or not. */
	bool IsComplete() const { return CurrentTime > CooldownTime; }
	
	/* Updates the Cooldown. */
	void Update(float DeltaTime) { CurrentTime += DeltaTime; }

	/* Returns the Calculated Cooldown. */
	float GetCooldownTime() const { return CooldownTime; }
private:
	/* The Ability for this Cooldown.*/
	TWeakObjectPtr<const UAblAbility> Ability;
	
	/* Current Time on the Cooldown.*/
	float CurrentTime;

	/* Total Cooldown time. */
	float CooldownTime;
};

UCLASS(ClassGroup = Able, hidecategories = (Internal, Tags, Activation, Collision), meta = (BlueprintSpawnableComponent, DisplayName = "Ability Component", ShortToolTip = "A component for playing active and passive abilities."))
class ABLECORE_API UAblAbilityComponent : public UActorComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
public:
	UAblAbilityComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// UActorComponent Overrides
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	////

	/* Returns whether or not this is a networked component/game or not. */
	bool IsNetworked() const;

	/* Returns if this owner is locally controlled. */
	bool IsOwnerLocallyControlled() const;

	/* Returns if this component is authoritative (only applies to networked games). */
	bool IsAuthoritative() const;

	/* Attempts to Branch to the provided Context. */
	UFUNCTION(BlueprintPure, Category="Able|Ability")
	EAblAbilityStartResult BranchAbility(UAblAbilityContext* Context);

	/* Runs the Context through the activate logic and returns the result, does not actually activate the Ability. */
	UFUNCTION(BlueprintPure, Category="Able|Ability")
	EAblAbilityStartResult CanActivateAbility(UAblAbilityContext* Context) const;
	
	/* Activates the ability (does call into CanActivateAbility to make sure the call is valid). */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability")
	EAblAbilityStartResult ActivateAbility(UAblAbilityContext* Context);

	/* Returns whether or not the component has an active Ability being played. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool IsPlayingAbility() const { return m_ActiveAbilityInstance != nullptr; }

	/* If you've set up your Animation Blueprint to use the AbilityAnimPlayer Graph Node, this method will tell if you should transition to that node. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool HasAbilityAnimation() const;

	/* Returns whether or not an Ability is marked as being on Cooldown. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool IsAbilityOnCooldown(const UAblAbility* Ability) const;

	/* Returns true if the passive ability passed in is currently active, or not. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool IsPassiveActive(const UAblAbility* Ability) const;

	/* If you need the active Ability, use this method. The one below requires a const_cast because BPs can't take Const pointers. */
	const UAblAbility* GetConstActiveAbility() const;

	/* Returns current Active Ability (if there is one). This can return null. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	UAblAbility* GetActiveAbility() const;

	/* Cancels the passed in Ability, if it's currently being played. Accepts both active and passive Abilities. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability")
	void CancelAbility(const UAblAbility* Ability, EAblAbilityTaskResult ResultToUse);

	/* Returns the total number of active passive abilities on this component. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability")
	int32 GetTotalNumberOfPassives() const { return m_PassiveAbilityInstances.Num(); }

	/* Returns the specific stack count for the passed in passive ability, or 0 if not found. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	int32 GetCurrentStackCountForPassiveAbility(const UAblAbility* Ability) const;

	/* Populates the passed in array with all the passive abilities currently active on this component. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	void GetCurrentPassiveAbilities(TArray<UAblAbility*>& OutPassives) const;

	/* Returns a value between 0 - 1 which represents how far along the cooldown is. 
	 * You can multiply this value by the Cooldown total to get the actual cooldown value in seconds.*/
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	float GetAbilityCooldownRatio(const UAblAbility* Ability) const;

	/* Returns the Cooldown that was calculated for this Ability when it was executed. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	float GetAbilityCooldownTotal(const UAblAbility* Ability) const;

	/* Sets the Branch context which is instantiated next tick. */
	void QueueContext(UAblAbilityContext* Context, EAblAbilityTaskResult ResultToUse);

	/* Returns the Gameplay Tag Container. */
	const FGameplayTagContainer& GetGameplayTagContainer() const { return m_TagContainer; }
	
	/* Returns all Cooldowns, mutable. */
	TMap<uint32, FAblAbilityCooldown>& GetMutableCooldowns() { return m_ActiveCooldowns; }
	
	/* Returns all Cooldowns, non-mutable. */
	const TMap<uint32, FAblAbilityCooldown>& GetCooldowns() const {	return m_ActiveCooldowns; }

	/* Finds the appropriate ability and adds the provided array to the Context Target Actors. AllowDuplicates will prevent adding any duplicates to the Target array (or not). */
	void AddAdditionTargetsToContext(const TWeakObjectPtr<const UAblAbilityContext>& Context, const TArray<TWeakObjectPtr<AActor>>& AdditionalTargets, bool AllowDuplicates = false);

#if WITH_EDITOR
	/* Plays the provided Ability. */
	void PlayAbilityFromEditor(const UAblAbility* Ability);
	
	/* Returns the current Ability play time. */
	float GetCurrentAbilityTime() const;

	/* Sets the current Ability play time. */
	void SetAbilityTime(float NewTime);
#endif

	/* Adds a Gameplay Tag to our tag container. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability")
	void AddTag(const FGameplayTag Tag);

	/* Removes a Gameplay Tag from our tag container. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability")
	void RemoveTag(const FGameplayTag Tag);

	/* Returns true if we have the supplied tag in our tag container. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool HasTag(const FGameplayTag Tag) const;

	/* Returns true if we have any tags from the passed in container in our own container. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool MatchesAnyTag(const FGameplayTagContainer Container) const;

	/* Returns true if we have all the tags from the passed in container in our own container. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool MatchesAllTags(const FGameplayTagContainer Container) const;

	/* Returns true if we match the supplied query with our GameplayTag container. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	bool MatchesQuery(const FGameplayTagQuery Query) const;

	/* Returns a copy of the Gameplay Tag container. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability", DisplayName = "GetGameplayTagContainer")
	FGameplayTagContainer GetGameplayTagContainerBP() const { return m_TagContainer; }
	
	// IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const { TagContainer = m_TagContainer; }

	FOnAbilityStart& GetOnAbilityStart() { return m_AbilityStartDelegate; }

	FOnAbilityEnd& GetOnAbilityEnd() { return m_AbilityEndDelegate; }

	void SetAbilityAnimationNode(const FAnimNode_AbilityAnimPlayer* Node);
protected:
	// Server Methods
	
	/* Attempts to activates the Ability using the provided Context. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerActivateAbility(const FAblAbilityNetworkContext& Context);
	
	/* Attempts to branch the Ability using the provided Context. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBranchAbility(const FAblAbilityNetworkContext& Context);

	/* Attempts to Cancel the provided Ability with the result passed in. */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCancelAbility(uint32 AbilityNameHash, EAblAbilityTaskResult ResultToUse);
	////

	// Client Replication Notifications

	/* Called when the Ability is changed on the Server. */
	UFUNCTION()
	void OnServerActiveAbilityChanged();
	
	/* Called when the number of Passives (or some aspect of the passive) is changed on the Server. */
	UFUNCTION()
	void OnServerPassiveAbilitiesChanged();
	////

	/* Checks if this Component still needs to Tick each frame. */
	void CheckNeedsTick();

	/* Returns true if the Component is in the middle of an update. */
	bool IsProcessingUpdate() const { return m_IsProcessingUpdate; }

	/* Internal/Shared Logic between Server and Client for starting an Ability.  */
	EAblAbilityStartResult InternalStartAbility(UAblAbilityContext* Context);
	
	/* Internal/Shared Logic between Server and Client for canceling an Ability. */
	void InternalCancelAbility(const UAblAbility* Ability, EAblAbilityTaskResult ResultToUse);

	/* Adds a Cooldown for the Provided Ability. */
	void AddCooldownForAbility(const UAblAbility& Ability, const UAblAbilityContext& Context);
	
	/* Internal/Shared Logic between Server and Client for updating all Abilities. */
	void InternalUpdateAbility(UAblAbilityInstance* AbilityInstance, float DeltaTime);

	/* Attempts to Activate the provided Context and returns the result, does not actually activate the Ability. */
	EAblAbilityStartResult CanActivatePassiveAbility(UAblAbilityContext* Context) const;

	/* Attempts to Activate the provided Context and returns the result. */
	EAblAbilityStartResult ActivatePassiveAbility(UAblAbilityContext* Context);
	
	/* Cancels the current Active Ability using the provided result. */
	void CancelActiveAbility(EAblAbilityTaskResult ResultToUse);

	// Friend class to help call this method via Async task graph.
	friend class FAsyncAbilityCooldownUpdaterTask;

	/* Updates all Cooldowns using the provided DeltaTime. */
	void UpdateCooldowns(float DeltaTime);

	/* Helper method to deal with all the pending contexts we may have. */
	void HandlePendingContexts();
	////

	// Network Helper Methods

	/* Returns the Role of this Component as a String in a simple "Client" / "Server" text. */
	FText& GetNetworkRoleText() const;

	/* Returns true if the provided Ability is allowed to run in this realm. */
	bool IsAbilityRealmAllowed(const UAblAbility& Ability) const;

	/* Updates the server representation of current passive Abilities (to be replicated to the client). */
	void UpdateServerPassiveAbilities();

	/* Updates the server representation of the current active Ability (to be replicated to the client). */
	void UpdateServerActiveAbility();
	////
protected:
	/* Our Active Ability Instance. */
	UPROPERTY(Transient)
	UAblAbilityInstance* m_ActiveAbilityInstance;

	/* Our Passive Ability Instances */
	UPROPERTY(Transient)
	TArray<UAblAbilityInstance*> m_PassiveAbilityInstances;

	/* Cached Settings Object for log reporting. */
	UPROPERTY(Transient)
	TWeakObjectPtr<const UAbleSettings> m_Settings;
	
	/* Active Cooldowns. */
	UPROPERTY(Transient)
	TMap<uint32, FAblAbilityCooldown> m_ActiveCooldowns;

	/* Pending Context */
	UPROPERTY(Transient)
	TArray<UAblAbilityContext*> m_PendingContext;

	/* How to treat the current Ability before switching to the new Context (success/normal, branch, interrupt). */
	UPROPERTY(Transient)
	TArray<TEnumAsByte<EAblAbilityTaskResult>> m_PendingResult;

	/* Contexts used in Async targeting, processed when they are completed. */
	UPROPERTY(Transient)
	TArray<UAblAbilityContext*> m_AsyncContexts; 

	/* Our Gameplay Tag Container */
	UPROPERTY(Transient)
	FGameplayTagContainer m_TagContainer;

	/* Boolean to track our update processing. */
	UPROPERTY(Transient)
	bool m_IsProcessingUpdate;


	FOnAbilityStart m_AbilityStartDelegate;

	FOnAbilityEnd m_AbilityEndDelegate;


	const FAnimNode_AbilityAnimPlayer* m_AbilityAnimationNode;

	FCriticalSection m_AbilityAnimNodeCS;

	// Network Fields
	// The Active Ability being played on the server.
	UPROPERTY(Transient, ReplicatedUsing = OnServerActiveAbilityChanged)
	FAblAbilityNetworkContext m_ServerActive;

	// The Active Passive Abilities being played on the server.
	UPROPERTY(Transient, ReplicatedUsing = OnServerPassiveAbilitiesChanged)
	TArray<FAblAbilityNetworkContext> m_ServerPassiveAbilities;
	///// 
};

#undef LOCTEXT_NAMESPACE
