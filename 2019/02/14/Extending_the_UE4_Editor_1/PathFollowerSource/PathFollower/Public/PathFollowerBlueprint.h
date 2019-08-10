// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "Engine/Blueprint.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

#include "pathFollowerBlueprint.generated.h"

UCLASS(BlueprintType)
class PATHFOLLOWER_API UPathFollowerBlueprint : public UBlueprint
{
	GENERATED_UCLASS_BODY()

public:
	/* Returns the UClass of our Ability Blueprints. */
	virtual UClass* GetBlueprintClass() const;

	virtual bool SupportedByDefaultBlueprintFactory() const { return false; }
	
	/** Returns the most base ability blueprint for a given blueprint (if it is inherited from another ability blueprint, returning null if only native / non-ability BP classes are it's parent) */
	static UPathFollowerBlueprint* FindRootGameplayAbilityBlueprint(UPathFollowerBlueprint* DerivedBlueprint);

	void AddComponent(UClass* NewComponentClass, FName ComponentName);
};
