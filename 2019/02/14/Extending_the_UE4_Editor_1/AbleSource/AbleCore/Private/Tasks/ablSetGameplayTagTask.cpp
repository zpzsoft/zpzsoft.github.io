// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablSetGameplayTagTask.h"

#include "ablAbilityComponent.h"
#include "AbleCorePrivate.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblSetGameplayTagTask::UAblSetGameplayTagTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_TagList(),
	m_RemoveOnEnd(false)
{

}

UAblSetGameplayTagTask::~UAblSetGameplayTagTask()
{

}

void UAblSetGameplayTagTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	TArray<TWeakObjectPtr<AActor>> TaskTargets;
	GetActorsForTask(Context, TaskTargets);

	for (const TWeakObjectPtr<AActor> & Actor : TaskTargets)
	{
		if (Actor.IsValid())
		{
			if (UAblAbilityComponent* AbilityComponent = Actor->FindComponentByClass<UAblAbilityComponent>())
			{
				for (const FGameplayTag& Tag : m_TagList)
				{
#if !(UE_BUILD_SHIPPING)
					if (IsVerbose())
					{
						PrintVerbose(FString::Printf(TEXT("Adding Tag %s to Actor %s."), *Actor->GetName(), *Tag.ToString()));
					}
#endif
					AbilityComponent->AddTag(Tag);
				}
			}
		}
	}
}

void UAblSetGameplayTagTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const
{
	Super::OnTaskEnd(Context, result);

	if (m_RemoveOnEnd)
	{
		TArray<TWeakObjectPtr<AActor>> TaskTargets;
		GetActorsForTask(Context, TaskTargets);

		for (const TWeakObjectPtr<AActor> & Actor : TaskTargets)
		{
			if (Actor.IsValid())
			{
				if (UAblAbilityComponent* AbilityComponent = Actor->FindComponentByClass<UAblAbilityComponent>())
				{
					for (const FGameplayTag& Tag : m_TagList)
					{
#if !(UE_BUILD_SHIPPING)
						if (IsVerbose())
						{
							PrintVerbose(FString::Printf(TEXT("Removing Tag %s from Actor %s."), *Actor->GetName(), *Tag.ToString()));
						}
#endif
						AbilityComponent->RemoveTag(Tag);
					}
				}
			}
		}
	}
}

TStatId UAblSetGameplayTagTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblSetGameplayTagTask, STATGROUP_Able);
}

#if WITH_EDITOR

FText UAblSetGameplayTagTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblPlaySetGameplayTagTaskFormat", "{0}: {1}");
	FString GameplayTagNames = TEXT("<none>");
	if (m_TagList.Num())
	{
		if (m_TagList.Num() == 1)
		{
			GameplayTagNames = m_TagList[0].ToString();
		}
		else
		{
			GameplayTagNames = FString::Printf(TEXT("%s, ... (%d Total Tags)"), *m_TagList[0].ToString(), m_TagList.Num());
		}

	}
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(GameplayTagNames));
}

#endif

#undef LOCTEXT_NAMESPACE
