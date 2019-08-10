// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "WorldCollision.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"
#include "ablAbilityContext.generated.h"

class UAblAbilityComponent;
class UAblAbility;

#define LOCTEXT_NAMESPACE "AblAbilityContext"

UENUM()
enum EAblAbilityTargetType
{
	Self UMETA(DisplayName="Self"),
	Owner UMETA(DisplayName="Owner"),
	Instigator UMETA(DisplayName="Instigator"),
	TargetActor UMETA(DisplayName="Target Actor"),
	Camera UMETA(DisplayName="Camera")
};

/* Struct used in Collision Queries. */
USTRUCT(BlueprintType)
struct ABLECORE_API FAblQueryResult
{
	GENERATED_USTRUCT_BODY();
public:
	FAblQueryResult() {};
	FAblQueryResult(const FOverlapResult& OverlapResult)
		: PrimitiveComponent(OverlapResult.GetComponent()),
		Actor(OverlapResult.GetActor())
	{

	}
	FAblQueryResult(const FHitResult& HitResult)
		:PrimitiveComponent(HitResult.GetComponent()),
		Actor(HitResult.GetActor())
	{

	}

	FAblQueryResult(UPrimitiveComponent* InPrimitiveComponent, AActor* InActor)
		: PrimitiveComponent(InPrimitiveComponent),
		Actor(InActor)
	{

	}

	/* Returns the Location of the Result. */
	FVector GetLocation() const;

	/* Returns the Transform of the Result. */
	void GetTransform(FTransform& OutTransform) const;

	/* Primitive Component from our Query result.*/
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Able|QueryResult", meta = (DisplayName = "Primitive Component"))
	TWeakObjectPtr<UPrimitiveComponent> PrimitiveComponent;

	/* Actor from our Query Result. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Able|QueryResult", meta = (DisplayName = "Actor"))
	TWeakObjectPtr<AActor> Actor;
};

/* Slightly more compact version of our normal Ability Context, for transfer across the wire. */
USTRUCT()
struct ABLECORE_API FAblAbilityNetworkContext
{
	GENERATED_USTRUCT_BODY();
public:
	FAblAbilityNetworkContext();
	FAblAbilityNetworkContext(const class UAblAbilityContext& Context);

	/* Resets this struct.*/
	void Reset();

	/* Returns true if this struct has valid properties. */
	bool IsValid() const;

	/* Sets the Stacks of this Context. */
	void SetCurrentStacks(int8 CurrentStacks) { m_CurrentStacks = CurrentStacks; }

	/* Returns the Ability contained by this Context.*/
	const TWeakObjectPtr<const UAblAbility>& GetAbility() const { return m_Ability; }
	
	/* Returns the Ability Component contained by this Context. */
	const TWeakObjectPtr<UAblAbilityComponent>& GetAbilityComponent() const { return m_AbilityComponent; }
	
	/* Returns the Owner contained by the Context. */
	const TWeakObjectPtr<AActor>& GetOwner() const { return m_Owner; }
	
	/* Returns the Instigator contained by the Context. */
	const TWeakObjectPtr<AActor>& GetInstigator() const { return m_Instigator; }

	/* Returns all Target actors contained by the Context. */
	const TArray<TWeakObjectPtr<AActor>>& GetTargetActors() const { return m_TargetActors; }

	/* Returns the Timestamp on when this Ability Context was created.*/
	float GetTimeStamp() const { return m_TimeStamp; }

	/* Returns the Current stack count of this Context. */
	int8 GetCurrentStack() const { return m_CurrentStacks; }
private:
	/* The Ability for this Context. */
	UPROPERTY()
	TWeakObjectPtr<const UAblAbility> m_Ability;

	/* The Ability Component for this Context. */
	UPROPERTY()
	TWeakObjectPtr<UAblAbilityComponent> m_AbilityComponent;

	/* The Owner Actor of this Context. */
	UPROPERTY()
	TWeakObjectPtr<AActor> m_Owner;

	/* The Instigator Actor of this Context. */
	UPROPERTY()
	TWeakObjectPtr<AActor> m_Instigator;

	/* The Target Actors of this Context. */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> m_TargetActors;

	/* The Current Ability Stacks of this Context. */
	UPROPERTY()
	int8 m_CurrentStacks;

	/* The Timestamp of this context. */
	UPROPERTY()
	float m_TimeStamp;
};

/* Ability Context, contains all the information needed during an Ability's execution. */
UCLASS(BlueprintType)
class ABLECORE_API UAblAbilityContext : public UObject
{
	GENERATED_BODY()
public:
	UAblAbilityContext(const FObjectInitializer& ObjectInitializer);
	virtual ~UAblAbilityContext();

	/* Uses the provided arguments to create an Ability Context. */
	static UAblAbilityContext* MakeContext(const UAblAbility* Ability, UAblAbilityComponent* AbilityComponent, AActor* Owner, AActor* Instigator);
	
	/* Creates an Ability Context from a Networked Ability Context. */
	static UAblAbilityContext* MakeContext(const FAblAbilityNetworkContext& NetworkContext);

	/* Allocates all required Task Scratchpads. */
	void AllocateScratchPads();

	/* Updates the time of this Context. */
	void UpdateTime(float DeltaTime);

	/* Returns the Scratchpad for the provided Task (if it has one). */
	class UAblAbilityTaskScratchPad* GetScratchPadForTask(const class UAblAbilityTask* Task) const;

	/* Returns Target Actor array, mutable. */
	TArray<TWeakObjectPtr<AActor>>& GetMutableTargetActors() { return m_TargetActors; }

	/* Returns the Ability Component executing this Context. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	UAblAbilityComponent* GetSelfAbilityComponent() const;

	/* Returns the Actor executing this Context. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	AActor* GetSelfActor() const;

	/* Returns the Owner Context Target Actor, if it exists. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	AActor* GetOwner() const { return m_Owner.Get(); }

	/* Returns the Instigator Context Target Actor, if it exists. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	AActor* GetInstigator() const { return m_Instigator.Get(); }

	/* Returns an array of all Target Context Target Actors, if they exist. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	const TArray<AActor*> GetTargetActors() const;

	/* Returns the Stack Count of this Ability. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	int32 GetCurrentStackCount() const;

	/* Sets the Stack Count of this Ability. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	void SetStackCount(int32 Stack);

	/* Returns the current time of this Ability. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	float GetCurrentTime() const { return m_CurrentTime; }

	/* Returns the current time of this Ability as a value between 0 and 1. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	float GetCurrentTimeRatio() const;


	/* Returns the last Delta time used to update this Ability. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Context")
	float GetLastDeltaTime() const { return m_LastDelta; }

	/* Returns the Ability contained in this Context. */
	const UAblAbility* GetAbility() const { return m_Ability; }

	/* Returns Target Actors Array directly (blueprint version requires a copy). */
	const TArray<TWeakObjectPtr<AActor>>& GetTargetActorsWeakPtr() const { return m_TargetActors; }

	/* Returns true if the Ability has found any targets. */
	bool HasAnyTargets() const { return m_TargetActors.Num() > 0; }

	/* Sets the Current Time of this Ability. */
	void SetCurrentTime(float Time) { m_CurrentTime = Time; }

	// Async Targeting Support
	/* Returns true if the context contains an Async handle for targeting. */
	bool HasValidAsyncHandle() const { return m_AsyncHandle._Handle != 0; }
	
	/* Sets the Async Handle.*/
	void SetAsyncHandle(FTraceHandle& InHandle) { m_AsyncHandle = InHandle; }
	
	/* Returns the Async Handle.*/
	const FTraceHandle& GetAsyncHandle() const { return m_AsyncHandle; }

	/* Sets the Async Targeting Transform. */
	void SetAsyncQueryTransform(const FTransform& Transform) { m_AsyncQueryTransform = Transform; }
	
	/* Returns the Async Targeting Transform. */
	const FTransform& GetAsyncQueryTransform() const { return m_AsyncQueryTransform; }
	//////
protected:
	/* The Ability being executed. */
	UPROPERTY(Transient)
	const UAblAbility* m_Ability;

	/* The Ability Component running this Ability. */
	UPROPERTY(Transient)
	UAblAbilityComponent* m_AbilityComponent;

	/* The Stack Count. */
	UPROPERTY(Transient)
	int32 m_StackCount;

	/* The Current Time of the Ability. */
	UPROPERTY(Transient)
	float m_CurrentTime;

	/* The Last delta value used to update the Ability. */
	UPROPERTY(Transient)
	float m_LastDelta;

	/* The "Owner" of this ability (may or may not be the same as the AbilityComponent owner).*/
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> m_Owner; 
	
	/* The Actor that caused this Ability to occur (if there is one).*/
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> m_Instigator;

	/* The Actor Targets of the Ability. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> m_TargetActors;

	/* Map of Task Unique IDs to ScratchPads. */
	UPROPERTY(Transient)
	TMap<uint32, class UAblAbilityTaskScratchPad*> m_TaskScratchPadMap;

	/* Used if our targeting call uses Async instead of Sync queries. */
	FTraceHandle m_AsyncHandle;

	/* Cached Transform used for our Async transform in case we need to do extra processing once our results come in. Currently only Cone check uses this. */
	UPROPERTY(Transient)
	FTransform m_AsyncQueryTransform;
};

#undef LOCTEXT_NAMESPACE