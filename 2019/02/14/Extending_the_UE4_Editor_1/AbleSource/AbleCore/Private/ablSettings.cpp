// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "ablSettings.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UAbleSettings::UAbleSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_EnableAsync(true),
	m_AllowAsyncAbilityUpdate(true),
	m_AllowAsyncCooldownUpdate(true),
	m_LogAbilityFailues(true),
	m_LogVerbose(true),
	m_EchoVerboseToScreen(false),
	m_VerboseScreenOutputLifetime(2.0f)
{

}

UAbleSettings::~UAbleSettings()
{

}

bool UAbleSettings::IsAsyncEnabled()
{
	static const UAbleSettings* Settings = nullptr;
	if (!Settings)
	{
		Settings = GetDefault<UAbleSettings>();
	}

	if (Settings)
	{
		// Should we bother trying to run Async on less than 2 cores? Doesn't seem worth it...
		return Settings->GetEnableAsync() && FPlatformMisc::NumberOfCores() > 2;
	}

	return false;
}
