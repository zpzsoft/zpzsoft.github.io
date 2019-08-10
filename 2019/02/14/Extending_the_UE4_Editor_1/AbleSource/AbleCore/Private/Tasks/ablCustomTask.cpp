// Copyright (c) 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablCustomTask.h"

#include "ablAbility.h"
#include "AbleCorePrivate.h"

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblCustomTaskScratchPad::UAblCustomTaskScratchPad()
{

}

UAblCustomTaskScratchPad::~UAblCustomTaskScratchPad()
{

}

UAblCustomTask::UAblCustomTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UAblCustomTask::~UAblCustomTask()
{

}

void UAblCustomTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	OnTaskStartBP(Context.Get());
}

void UAblCustomTask::OnTaskStartBP_Implementation(const UAblAbilityContext* Context) const
{

}

void UAblCustomTask::OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const
{
	OnTaskTickBP(Context.Get(), deltaTime);
}

void UAblCustomTask::OnTaskTickBP_Implementation(const UAblAbilityContext* Context, float DeltaTime) const
{

}

void UAblCustomTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const
{
	OnTaskEndBP(Context.Get(), result);
}

void UAblCustomTask::OnTaskEndBP_Implementation(const UAblAbilityContext* Context, const EAblAbilityTaskResult result) const
{

}

bool UAblCustomTask::IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	return IsDoneBP(Context.Get());
}

bool UAblCustomTask::IsDoneBP_Implementation(const UAblAbilityContext* Context) const
{
	return UAblAbilityTask::IsDone(TWeakObjectPtr<const UAblAbilityContext>(Context));
}

UAblAbilityTaskScratchPad* UAblCustomTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return CastChecked<UAblAbilityTaskScratchPad>(CreateScratchPadBP(Context.Get()));
}

UAblCustomTaskScratchPad* UAblCustomTask::GetScratchPad(UAblAbilityContext* Context) const
{
	return CastChecked<UAblCustomTaskScratchPad>(Context->GetScratchPadForTask(this));
}

UAblCustomTaskScratchPad* UAblCustomTask::CreateScratchPadBP_Implementation(UAblAbilityContext* Context) const
{
	return NewObject<UAblCustomTaskScratchPad>(Context);
}

bool UAblCustomTask::IsSingleFrame() const
{
	return IsSingleFrameBP();
}

bool UAblCustomTask::IsSingleFrameBP_Implementation() const
{
	return true;
}

EAblAbilityTaskRealm UAblCustomTask::GetTaskRealm() const
{
	return GetTaskRealmBP();
}

EAblAbilityTaskRealm UAblCustomTask::GetTaskRealmBP_Implementation() const
{
	return EAblAbilityTaskRealm::Client;
}

void UAblCustomTask::GetActorsForTaskBP(const UAblAbilityContext* Context, TArray<AActor*>& OutActorArray) const
{
	TArray<TWeakObjectPtr<AActor>> TempArray;
	GetActorsForTask(TWeakObjectPtr<const UAblAbilityContext>(Context), TempArray);

	// BP's don't support Containers of WeakObjectPtrs so we have to do a fun copy here... :/
	OutActorArray.Empty();
	for (TWeakObjectPtr<AActor>& FoundActor : TempArray)
	{
		if (FoundActor.IsValid())
		{
			OutActorArray.Add(FoundActor.Get());
		}
	}
}

TStatId UAblCustomTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblCustomTask, STATGROUP_Able);
}

FText UAblCustomTask::GetTaskCategoryBP_Implementation() const
{
	return LOCTEXT("AblCustomTaskCategory", "Blueprint|Custom");
}

FText UAblCustomTask::GetTaskNameBP_Implementation() const
{
	return FText::FromString(GetName());
}

FText UAblCustomTask::GetDescriptiveTaskNameBP_Implementation() const
{
	return LOCTEXT("AblCustomTaskDisplayName", "Custom");
}

FText UAblCustomTask::GetTaskDescriptionBP_Implementation() const
{
	return FText::FromString(GetName());
}

FLinearColor UAblCustomTask::GetTaskColorBP_Implementation() const
{
	return FLinearColor::White;
}
#if WITH_EDITOR

FText UAblCustomTask::GetTaskCategory() const
{
	return GetTaskCategoryBP();
}

FText UAblCustomTask::GetTaskName() const
{
	return GetTaskNameBP();
}

FText UAblCustomTask::GetDescriptiveTaskName() const
{
	return GetDescriptiveTaskNameBP();
}

FText UAblCustomTask::GetTaskDescription() const
{
	return GetTaskDescriptionBP();
}

FLinearColor UAblCustomTask::GetTaskColor() const
{
	return GetTaskColorBP();
}

#endif

#undef LOCTEXT_NAMESPACE