// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablPlayAbilityTask.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "ablAbilityContext.h"
#include "AbleCorePrivate.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblPlayAbilityTask::UAblPlayAbilityTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Ability(),
	m_Owner(EAblAbilityTargetType::Owner),
	m_Instigator(EAblAbilityTargetType::Self)
{

}

UAblPlayAbilityTask::~UAblPlayAbilityTask()
{

}

void UAblPlayAbilityTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	check(Context.IsValid());

	Super::OnTaskStart(Context);

	if (const UAblAbility* Ability = Cast<const UAblAbility>(m_Ability->GetDefaultObject()))
	{
		TArray<TWeakObjectPtr<AActor>> TaskTargets;
		GetActorsForTask(Context, TaskTargets);

		AActor* InstigatorActor = GetSingleActorFromTargetType(Context, m_Instigator);
		AActor* OwnerActor = GetSingleActorFromTargetType(Context, m_Owner);
		UAblAbilityComponent* AbilityComponent = nullptr;
		
		for (const TWeakObjectPtr<AActor>& TaskTarget : TaskTargets)
		{
			if (TaskTarget.IsValid())
			{
				// Target Actor with GetSingleActorFromTargetType always returns the first entry, so we need to
				// update the value as we work if we're set to use that Target Type.
				if (m_Instigator.GetValue() == EAblAbilityTargetType::TargetActor)
				{
					InstigatorActor = TaskTarget.Get();
				}

				if (m_Owner.GetValue() == EAblAbilityTargetType::TargetActor)
				{
					OwnerActor = TaskTarget.Get();
				}

				AbilityComponent = TaskTarget->FindComponentByClass<UAblAbilityComponent>();
				if (AbilityComponent)
				{
#if !(UE_BUILD_SHIPPING)
					if (IsVerbose())
					{
						PrintVerbose(FString::Printf(TEXT("Calling ActivateAbility with Actor %s with Ability %s, Owner %s, Instigator %s."), *TaskTarget->GetName(), 
							*Ability->GetDisplayName(), InstigatorActor ? *InstigatorActor->GetName() : *FString("None"), OwnerActor ? *OwnerActor->GetName() : *FString("None")));
					}
#endif
					AbilityComponent->ActivateAbility(UAblAbilityContext::MakeContext(Ability, AbilityComponent, OwnerActor, InstigatorActor));
				}
			}
		}
	}
	else
	{
		UE_LOG(LogAble, Warning, TEXT("No Ability set for PlayAbilityTask in Ability [%s]"), *Context->GetAbility()->GetDisplayName());
		return;
	}
}

TStatId UAblPlayAbilityTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblPlayAbilityTask, STATGROUP_Able);
}

#if WITH_EDITOR

FText UAblPlayAbilityTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblPlayAbilityTaskFormat", "{0}: {1}");
	FString AbilityName = TEXT("<null>");
	if (*m_Ability)
	{
		if (UAblAbility* Ability = Cast<UAblAbility>(m_Ability->GetDefaultObject()))
		{
			AbilityName = Ability->GetDisplayName();
		}
	}

	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(AbilityName));
}

#endif

#undef LOCTEXT_NAMESPACE