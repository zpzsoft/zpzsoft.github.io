// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "ablSettings.generated.h"
/**
* Implements the settings for the Able Toolkit
*/
UCLASS(config = Engine, defaultconfig)
class ABLECORE_API UAbleSettings : public UObject
{
	GENERATED_BODY()

public:
	UAbleSettings(const FObjectInitializer& ObjectInitializer);
	virtual ~UAbleSettings();

	/* Returns true if Async is enabled (and allowed on this platform). */
	static bool IsAsyncEnabled();

	/* Returns true if Async is Enabled by the user. */
	FORCEINLINE bool GetEnableAsync() const { return m_EnableAsync; }

	/* Returns true if the Async Update for Abilities is allowed. */
	FORCEINLINE bool GetAllowAbilityAsyncUpdate() const { return m_AllowAsyncAbilityUpdate; }
	
	/* Returns true if the Async Cooldown Update is allowed. */
	FORCEINLINE bool GetAllowAsyncCooldownUpdate() const { return m_AllowAsyncCooldownUpdate; }

	/* Returns true if we should log all Ability failures. */
	FORCEINLINE bool GetLogAbilityFailures() const { return m_LogAbilityFailues; }
	
	/* Returns true if we should log all verbose output. */
	FORCEINLINE bool GetLogVerbose() const { return m_LogVerbose; }

	/* Returns true if we should echo our verbose output to screen. */
	FORCEINLINE bool GetEchoVerboseToScreen() const { return m_EchoVerboseToScreen; }

	/* Returns the lifetime of a Verbose screen message, in seconds. */
	FORCEINLINE float GetVerboseScreenLifetime() const { return m_VerboseScreenOutputLifetime; }
private:
	/* If true, Able will attempt to use Async options when available and hardware permits it. */
	UPROPERTY(config, EditAnywhere, Category = Ability, meta=(DisplayName="Enable Async"))
	bool m_EnableAsync;

	/* If true, allows abilities to use the async task graph to perform updates (when available). This can increase performance during heavy ability usage. */
	UPROPERTY(config, EditAnywhere, Category = Ability, meta=(DisplayName="Allow Async Ability Update", EditCondition=m_EnableAsync))
	bool m_AllowAsyncAbilityUpdate;

	/* If true, Ability components will launch a separate task to update their active cooldowns. This can increase performance during heavy ability usage. */
	UPROPERTY(config, EditAnywhere, Category = Ability, meta = (DisplayName = "Allow Async Cooldown Update", EditCondition = m_EnableAsync))
	bool m_AllowAsyncCooldownUpdate;

	/* If true, we write out Ability name and failure codes when they fail to play (cooldown, custom check, etc). */
	UPROPERTY(config, EditAnywhere, Category = Ability, meta = (DisplayName = "Log Ability Failures"))
	bool m_LogAbilityFailues;

	/* If true, Abilities/Tasks marked as Verbose will echo their information to the log. Can be used with screen output. */
	UPROPERTY(config, EditAnywhere, Category = Ability, meta = (DisplayName = "Log Verbose Output"))
	bool m_LogVerbose;

	/* If true, Abilities/Tasks marked as Verbose will echo their information to the screen. Can be used with log output. */
	UPROPERTY(config, EditAnywhere, Category = Ability, meta = (DisplayName = "Verbose Output to Screen"))
	bool m_EchoVerboseToScreen;

	/* How long, in seconds, to display the Verbose output on the screen. */
	UPROPERTY(config, EditAnywhere, Category = Ability, meta = (DisplayName = "Verbose Screen Lifetime", EditCondition = m_EchoVerboseToScreen))
	float m_VerboseScreenOutputLifetime;
};
