// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablBranchTask.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "AbleCorePrivate.h"
#include "Tasks/ablBranchCondition.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblBranchTaskScratchPad::UAblBranchTaskScratchPad()
	: BranchAbility(nullptr)
{

}

UAblBranchTaskScratchPad::~UAblBranchTaskScratchPad()
{

}

UAblBranchTask::UAblBranchTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_BranchAbility(nullptr),
	m_DynamicBranchAbility(false),
	m_DynamicBranchEventName(NAME_None),
	m_MustPassAllConditions(false),
	m_CopyTargetsOnBranch(false)
{

}

UAblBranchTask::~UAblBranchTask()
{

}

void UAblBranchTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	UAblBranchTaskScratchPad* ScratchPad = Cast<UAblBranchTaskScratchPad>(Context->GetScratchPadForTask(this));
	check(ScratchPad);

	ScratchPad->BranchAbility = Context->GetAbility();

	if (CheckBranchCondition(Context))
	{

#if !(UE_BUILD_SHIPPING)
		if (IsVerbose())
		{
			PrintVerbose(FString::Printf(TEXT("Conditions passed. Branching to Ability %s"), *(m_BranchAbility->GetName())));
		}
#endif
		UAblAbility* BranchToAbility = m_DynamicBranchAbility ? Context->GetAbility()->OnGetBranchAbility(Context.Get(), m_DynamicBranchEventName) : Cast<UAblAbility>(m_BranchAbility->GetDefaultObject());
		if (BranchToAbility)
		{
			UAblAbilityContext* NewContext = UAblAbilityContext::MakeContext(BranchToAbility, Context->GetSelfAbilityComponent(), Context->GetOwner(), Context->GetInstigator());

			if (m_CopyTargetsOnBranch)
			{
				NewContext->GetMutableTargetActors().Append(Context->GetTargetActors());
			}

			Context->GetSelfAbilityComponent()->BranchAbility(NewContext);
		}
#if !(UE_BUILD_SHIPPING)
		else if (IsVerbose())
		{
			PrintVerbose(FString::Printf(TEXT("Branching Failed. Branch Ability was null.")));
		}
#endif
	}
}

void UAblBranchTask::OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const
{
	Super::OnTaskTick(Context, deltaTime);

	if (CheckBranchCondition(Context))
	{

#if !(UE_BUILD_SHIPPING)
		if (IsVerbose())
		{
			PrintVerbose(FString::Printf(TEXT("Conditions passed. Branching to Ability %s"), *(m_BranchAbility->GetName())));
		}
#endif
		UAblAbility* BranchToAbility = m_DynamicBranchAbility ? Context->GetAbility()->OnGetBranchAbility(Context.Get(), m_DynamicBranchEventName) : Cast<UAblAbility>(m_BranchAbility->GetDefaultObject());
		if (BranchToAbility)
		{
			UAblAbilityContext* NewContext = UAblAbilityContext::MakeContext(BranchToAbility, Context->GetSelfAbilityComponent(), Context->GetOwner(), Context->GetInstigator());

			if (m_CopyTargetsOnBranch)
			{
				NewContext->GetMutableTargetActors().Append(Context->GetTargetActors());
			}

			Context->GetSelfAbilityComponent()->BranchAbility(NewContext);
		}
#if !(UE_BUILD_SHIPPING)
		else if (IsVerbose())
		{
			PrintVerbose(FString::Printf(TEXT("Branching Failed. Branch Ability was null.")));
		}
#endif
	}
}

UAblAbilityTaskScratchPad* UAblBranchTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return NewObject<UAblBranchTaskScratchPad>(Context.Get());
}

TStatId UAblBranchTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblBranchTask, STATGROUP_Able);
}

bool UAblBranchTask::CheckBranchCondition(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	UAblBranchTaskScratchPad* ScratchPad = Cast<UAblBranchTaskScratchPad>(Context->GetScratchPadForTask(this));
	check(ScratchPad);

	bool Result = false;
	for (UAblBranchCondition* Condition : m_Conditions)
	{
		Result = Condition->CheckCondition(Context, *ScratchPad);
		if (Condition->IsNegated())
		{
			Result = !Result;
		}

#if !(UE_BUILD_SHIPPING)
		if (IsVerbose())
		{
			PrintVerbose(FString::Printf(TEXT("Condition %s returned %s."), *Condition->GetName(), Result ? *FString("True") : *FString("False")));
		}
#endif

		// Check our early out cases.
		if (m_MustPassAllConditions && !Result)
		{
			// Failed
			break;
		}
		else if (!m_MustPassAllConditions && Result)
		{
			// Success
			break;
		}
	}

	return Result;
}

#if WITH_EDITOR

FText UAblBranchTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblBranchTaskFormat", "{0}: {1}");
	FString AbilityName = ("<null>");
	if (m_DynamicBranchAbility)
	{
		AbilityName = ("Dynamic");
	}
	else if (*m_BranchAbility)
	{
		if (UAblAbility* Ability = Cast<UAblAbility>(m_BranchAbility->GetDefaultObject()))
		{
			AbilityName = Ability->GetDisplayName();
		}
	}

	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(AbilityName));
}

#endif

#undef LOCTEXT_NAMESPACE
