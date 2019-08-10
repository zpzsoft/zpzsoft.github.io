// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablDamageEventTask.h"

#include "ablAbility.h"
#include "ablAbilityUtilities.h"
#include "AbleCorePrivate.h"
#include "ablSettings.h"
#include "Async/Async.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblDamageEventTask::UAblDamageEventTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Damage(1.0f),
	m_DamageSource(EAblAbilityTargetType::Self),
	m_EventName(NAME_None),
	m_TaskRealm(EAblAbilityTaskRealm::ClientAndServer)
{

}

UAblDamageEventTask::~UAblDamageEventTask()
{

}

void UAblDamageEventTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	check(Context.IsValid());

	Super::OnTaskStart(Context);

	AActor* DamageSource = GetSingleActorFromTargetType(Context, m_DamageSource);
		
	TArray<TWeakObjectPtr<AActor>> DamageTargets;
	GetDamageTargets(Context, DamageTargets);

#if !(UE_BUILD_SHIPPING)
	if (IsVerbose())
	{
		PrintVerbose(FString::Printf(TEXT("Executing Damage calculations for %d targets."), DamageTargets.Num()));
	}
#endif

	if (DamageTargets.Num())
	{
		TArray<float> DamageValues;
		DamageValues.Reserve(DamageTargets.Num());

		if (m_UseAsyncCalculate && UAbleSettings::IsAsyncEnabled())
		{
			TArray<TFuture<float>> CalculateTasks;
			CalculateTasks.Reserve(DamageTargets.Num());

			for (TWeakObjectPtr<AActor>& DamageTarget : DamageTargets)
			{
				// Copy over to local variables for the Lambda.
				float BaseDamage = m_Damage;
				FName EventName = m_EventName;

				CalculateTasks.Add(Async<float>(EAsyncExecution::TaskGraph, [&Context, BaseDamage, EventName, &DamageTarget]
				{
					if (Context.IsValid())
					{
						return Context->GetAbility()->CalculateDamageForActor(Context.Get(), EventName, BaseDamage, DamageTarget.Get());
					}

					return BaseDamage;
				}));
			}

			for (TFuture<float>& Future : CalculateTasks)
			{
				if (!Future.IsReady())
				{
					static const FTimespan OneMillisecond = FTimespan::FromMilliseconds(1.0);
					Future.WaitFor(OneMillisecond);
				}

				// Copy the Damage over.
				DamageValues.Add(Future.Get());
			}
		}
		else
		{
			for (TWeakObjectPtr<AActor>& DamageTarget : DamageTargets)
			{
				DamageValues.Add(Context->GetAbility()->CalculateDamageForActor(Context.Get(), m_EventName, m_Damage, DamageTarget.Get()));

#if !(UE_BUILD_SHIPPING)
				if (IsVerbose() && DamageTarget.IsValid())
				{
					PrintVerbose(FString::Printf(TEXT("CalculateDamageForActor with Actor %s and Base Damage %4.2f returned %4.2f."), *DamageTarget->GetName(), m_Damage, DamageValues[DamageValues.Num() - 1]));
				}
#endif
			}
		}

		check(DamageTargets.Num() == DamageValues.Num());

		FDamageEvent EmptyEvent;
		for (int32 i = 0; i < DamageTargets.Num(); ++i)
		{
			if (DamageTargets[i].IsValid())
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Applying %4.2f damage to %s."), DamageValues[i], *DamageTargets[i]->GetName()));
				}
#endif
				DamageTargets[i]->TakeDamage(DamageValues[i], EmptyEvent, nullptr, DamageSource);
			}
		}
	}
}

TStatId UAblDamageEventTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblDamageEventTask, STATGROUP_Able);
}

void UAblDamageEventTask::GetDamageTargets(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<TWeakObjectPtr<AActor>>& OutArray) const
{
	for (const EAblAbilityTargetType& Target : m_DamageTargets)
	{
		switch (Target)
		{
			case EAblAbilityTargetType::Self:
			case EAblAbilityTargetType::Owner:
			case EAblAbilityTargetType::Instigator:
			{
				if (AActor* Actor = GetSingleActorFromTargetType(Context, Target))
				{
					OutArray.Add(Actor);
				}
			}
			break;
			case EAblAbilityTargetType::TargetActor:
			{
				const TArray<TWeakObjectPtr<AActor>>& UnfilteredTargets = Context->GetTargetActorsWeakPtr();
				for (const TWeakObjectPtr<AActor>& TempTarget : UnfilteredTargets)
				{
					if (IsTaskValidForActor(TempTarget.Get()))
					{
						OutArray.Add(TempTarget);
					}
				}
			}
			break;
			default:
			break;
		}
	}
}

#if WITH_EDITOR

FText UAblDamageEventTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblDamageEventTaskFormat", "{0}: {1}");
	FString TargetNames = TEXT("<None>");
	if (m_DamageTargets.Num())
	{
		TargetNames = FAbleLogHelper::GetTargetTargetEnumAsString(m_DamageTargets[0]);
		if (m_DamageTargets.Num() > 1)
		{
			TargetNames.Append(FString::Printf(TEXT(", ... (%d Total Targets)"), m_DamageTargets.Num()));
		}

	}
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(TargetNames));
}

#endif

#undef LOCTEXT_NAMESPACE
