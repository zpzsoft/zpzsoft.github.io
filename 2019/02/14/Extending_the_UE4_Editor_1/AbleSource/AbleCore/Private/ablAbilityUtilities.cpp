// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "ablAbilityUtilities.h"

#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

const FString FAbleLogHelper::GetResultEnumAsString(EAblAbilityStartResult Result)
{
	if (UEnum* EnumClass = FindObject<UEnum>(ANY_PACKAGE, TEXT("EAblAbilityStartResult"), true))
	{
#if WITH_EDITOR
		return EnumClass->GetDisplayNameTextByValue(static_cast<int32>(Result)).ToString();
#else
		return EnumClass->GetNameStringByValue(static_cast<int32>(Result));
#endif
	}

	return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();
}

const FString FAbleLogHelper::GetTaskResultEnumAsString(EAblAbilityTaskResult Result)
{
	if (UEnum* EnumClass = FindObject<UEnum>(ANY_PACKAGE, TEXT("EAblAbilityTaskResult"), true))
	{
#if WITH_EDITOR
		return EnumClass->GetDisplayNameTextByValue(static_cast<int32>(Result)).ToString();
#else
		return EnumClass->GetNameStringByValue(static_cast<int32>(Result));
#endif
	}

	return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();
}

const FString FAbleLogHelper::GetTargetTargetEnumAsString(EAblAbilityTargetType Result)
{
	if (UEnum* EnumClass = FindObject<UEnum>(ANY_PACKAGE, TEXT("EAblAbilityTargetType"), true))
	{
#if WITH_EDITOR
		return EnumClass->GetDisplayNameTextByValue(static_cast<int32>(Result)).ToString();
#else
		return EnumClass->GetNameStringByValue(static_cast<int32>(Result));
#endif
	}

	return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();
}

const FString FAbleLogHelper::GetCollisionResponseEnumAsString(ECollisionResponse Response)
{
	if (UEnum* EnumClass = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECollisionResponse"), true))
	{
#if WITH_EDITOR
		return EnumClass->GetDisplayNameTextByValue(static_cast<int32>(Response)).ToString();
#else
		return EnumClass->GetNameStringByValue(static_cast<int32>(Response));
#endif
	}

	return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();
}

const FString FAbleLogHelper::GetCollisionChannelEnumAsString(ECollisionChannel Channel)
{
	if (UEnum* EnumClass = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECollisionChannel"), true))
	{
#if WITH_EDITOR
		return EnumClass->GetDisplayNameTextByValue(static_cast<int32>(Channel)).ToString();
#else
		return EnumClass->GetNameStringByValue(static_cast<int32>(Channel));
#endif
	}

	return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();
}

const TArray<FKey> FAblAbilityUtilities::GetKeysForInputAction(const FName& InputAction)
{
	TArray<FKey> ReturnValue;
	const UInputSettings* InputSettings = GetDefault<UInputSettings>();

	if (InputSettings)
	{
		const FName LocalCopy = InputAction;
		for (const FInputActionKeyMapping& ActionMapping : InputSettings->ActionMappings)
		{
			if (ActionMapping.ActionName == InputAction)
			{
				ReturnValue.Add(ActionMapping.Key);
			}
		}
	}

	return ReturnValue;
}


