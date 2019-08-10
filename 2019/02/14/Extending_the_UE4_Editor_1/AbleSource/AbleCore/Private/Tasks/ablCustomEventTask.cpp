// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablCustomEventTask.h"

#include "ablAbility.h"
#include "AbleCorePrivate.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblCustomEventTask::UAblCustomEventTask(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer),
	m_EventName(NAME_None)
{

}

UAblCustomEventTask::~UAblCustomEventTask()
{

}

void UAblCustomEventTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

#if !(UE_BUILD_SHIPPING)
	if (IsVerbose())
	{
		PrintVerbose(FString::Printf(TEXT("Firing Custom Event %s."), *m_EventName.ToString()));
	}
#endif

	// Call our parent.
	Context->GetAbility()->OnCustomEvent(Context.Get(), m_EventName);
}

TStatId UAblCustomEventTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblCustomEventTask, STATGROUP_Able);
}

#if WITH_EDITOR

FText UAblCustomEventTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblCustomEventTaskFormat", "{0}: {1}");
	FString EventName = m_EventName.IsNone() ? ("<none>") : m_EventName.ToString();
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(EventName));
}

#endif

#undef LOCTEXT_NAMESPACE
