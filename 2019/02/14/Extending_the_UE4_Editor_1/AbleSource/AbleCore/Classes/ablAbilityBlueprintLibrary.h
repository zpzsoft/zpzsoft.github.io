// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Tasks/ablCustomTask.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

#include "ablAbilityBlueprintLibrary.generated.h"

#define LOCTEXT_NAMESPACE "AbleCore"

class UAblAbility;
class UAblAbilityInstance;
class UAblAbilityContext;
class UAblCustomTaskScratchPad;

UCLASS()
class ABLECORE_API UAblAbilityBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/* Returns an instance (CDO) of the provided Ability Class. */
	UFUNCTION(BlueprintPure, Category="Able|Ability")
	static UAblAbility* GetAbilityObjectFromClass(UClass* Class);

	/* Activates the Ability using the provided context and returns the result. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability")
	static EAblAbilityStartResult ActivateAbility(UAblAbilityContext* Context);

	/* Creates an Ability Context with the provided information. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability|Context")
	static UAblAbilityContext* CreateAbilityContext(const UAblAbility* Ability, UAblAbilityComponent* AbilityComponent, AActor* Owner, AActor* Instigator);

	/* Returns true if the provided start result is considered successful. */
	UFUNCTION(BlueprintPure, Category = "Able|Ability")
	static bool IsSuccessfulStartResult(EAblAbilityStartResult Result);

	/* Returns whether debugging drawing of collision queries is enabled or not.  */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Debug")
	static bool GetDrawCollisionQueries();

	/* Toggles the viewing of collision queries within Able. Returns the new value. */
	UFUNCTION(BlueprintCallable, Category = "Able|Ability|Debug")
	static bool SetDrawCollisionQueries(bool Enable);

	/* Creates a ScratchPad based on the UAblCustomScratchPad, for use by Custom Tasks. */
	UFUNCTION(BlueprintCallable, Category = "Able|Custom Task")
	static UAblCustomTaskScratchPad* CreateCustomScratchPad(UAblAbilityContext* Context, TSubclassOf<UAblCustomTaskScratchPad> ScratchPadClass);
};

#undef LOCTEXT_NAMESPACE
