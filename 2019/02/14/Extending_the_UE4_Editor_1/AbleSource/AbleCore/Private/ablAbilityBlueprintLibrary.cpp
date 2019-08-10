// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "ablAbilityBlueprintLibrary.h"

#include "ablAbility.h"
#include "ablAbilityContext.h"
#include "ablAbilityComponent.h"
#include "ablAbilityDebug.h"
#include "Tasks/ablCustomTask.h"
#include "AbleCorePrivate.h"

#define LOCTEXT_NAMESPACE "AbleCore"

UAblAbility* UAblAbilityBlueprintLibrary::GetAbilityObjectFromClass(UClass* Class)
{
	if (Class && Class->IsChildOf<UAblAbility>())
	{
		if (UAblAbility* AbilityObject = Class->GetDefaultObject<UAblAbility>())
		{
			return AbilityObject;
		}
		else
		{
			UE_LOG(LogAble, Error, TEXT("GetAbilityObjectFromClass failed to get default object for class %s"), *Class->GetName());
		}
	}
	else
	{
		UE_LOG(LogAble, Error, TEXT("GetAbilityObjectFromClass was passed a null/invalid class. Check your parameters!"));
	}


	return nullptr;
}

EAblAbilityStartResult UAblAbilityBlueprintLibrary::ActivateAbility(UAblAbilityContext* Context)
{
	if (Context)
	{
		if (UAblAbilityComponent* AbilityComponent = Context->GetSelfAbilityComponent())
		{
			return AbilityComponent->ActivateAbility(Context);
		}
		else
		{
			UE_LOG(LogAble, Error, TEXT("ActivateAbility was passed a context with a null ability component. Check your parameters!"));
		}
	}
	else
	{
		UE_LOG(LogAble, Error, TEXT("ActivateAbility was passed a null context. Check your parameters!"));
	}


	return EAblAbilityStartResult::InternalSystemsError;
}

UAblAbilityContext* UAblAbilityBlueprintLibrary::CreateAbilityContext(const UAblAbility* Ability, UAblAbilityComponent* AbilityComponent, AActor* Owner, AActor* Instigator)
{
	return UAblAbilityContext::MakeContext(Ability, AbilityComponent, Owner, Instigator);
}

bool UAblAbilityBlueprintLibrary::IsSuccessfulStartResult(EAblAbilityStartResult Result)
{
	// Async Processing is technically a Success as we can't determine whether it passed or failed for a frame.
	return Result == EAblAbilityStartResult::Success ||
		Result == EAblAbilityStartResult::AsyncProcessing;
}

bool UAblAbilityBlueprintLibrary::GetDrawCollisionQueries()
{
#if !UE_BUILD_SHIPPING
	return FAblAbilityDebug::ShouldDrawQueries();
#else
	return false;
#endif
}

bool UAblAbilityBlueprintLibrary::SetDrawCollisionQueries(bool Enable)
{
#if !UE_BUILD_SHIPPING
	FAblAbilityDebug::EnableDrawQueries(Enable);

	return FAblAbilityDebug::ShouldDrawQueries();
#else
	return false;
#endif
}

UAblCustomTaskScratchPad* UAblAbilityBlueprintLibrary::CreateCustomScratchPad(UAblAbilityContext* Context, TSubclassOf<UAblCustomTaskScratchPad> ScratchPadClass)
{
	if (*ScratchPadClass)
	{
		return NewObject<UAblCustomTaskScratchPad>(Context, *ScratchPadClass);
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE