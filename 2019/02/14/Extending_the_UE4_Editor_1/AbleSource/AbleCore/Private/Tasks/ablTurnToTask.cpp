// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablTurnToTask.h"

#include "ablAbility.h"
#include "AbleCorePrivate.h"

#if (!UE_BUILD_SHIPPING)
#include "ablAbilityUtilities.h"
#endif

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblTurnToTaskScratchPad::UAblTurnToTaskScratchPad()
{

}

UAblTurnToTaskScratchPad::~UAblTurnToTaskScratchPad()
{

}

UAblTurnToTask::UAblTurnToTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_RotationTarget(EAblAbilityTargetType::TargetActor),
	m_TrackTarget(false),
	m_SetYaw(true),
	m_SetPitch(false)
{

}

UAblTurnToTask::~UAblTurnToTask()
{

}

void UAblTurnToTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	UAblTurnToTaskScratchPad* ScratchPad = Cast<UAblTurnToTaskScratchPad>(Context->GetScratchPadForTask(this));
	check(ScratchPad);

	ScratchPad->TurningBlend = m_Blend;

	if (AActor* TargetActor = GetSingleActorFromTargetType(Context, m_RotationTarget.GetValue()))
	{
		TArray<TWeakObjectPtr<AActor>> TaskTargets;
		GetActorsForTask(Context, TaskTargets);

		for (TWeakObjectPtr<AActor>& TurnTarget : TaskTargets)
		{
			FRotator TargetRotation = GetTargetRotation(TurnTarget.Get(), TargetActor);
#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Setting up turning for Actor %s with a Target turn of %s."), *TurnTarget->GetName(), *TargetRotation.ToCompactString()));
			}
#endif
			ScratchPad->InProgressTurn.Add(FTurnToTaskEntry(TurnTarget.Get(), TargetRotation));
		}
	}

}

void UAblTurnToTask::OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const
{
	Super::OnTaskTick(Context, deltaTime);

	UAblTurnToTaskScratchPad* ScratchPad = Cast<UAblTurnToTaskScratchPad>(Context->GetScratchPadForTask(this));
	check(ScratchPad);

	ScratchPad->TurningBlend.Update(deltaTime);
	const float BlendingValue = ScratchPad->TurningBlend.GetBlendedValue();
	AActor* TargetActor = GetSingleActorFromTargetType(Context, m_RotationTarget.GetValue());

	for (FTurnToTaskEntry& Entry : ScratchPad->InProgressTurn)
	{
		if (Entry.Actor.IsValid())
		{
			if (m_TrackTarget && TargetActor)
			{
				// Update our Target rotation.
				Entry.Target = GetTargetRotation(Entry.Actor.Get(), TargetActor);
			}

			FRotator LerpedRotation = FMath::Lerp(Entry.Actor->GetActorRotation(), Entry.Target, BlendingValue);
#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Setting Actor %s rotation to %s ."), *Entry.Actor->GetName(), *LerpedRotation.ToCompactString()));
			}
#endif
			Entry.Actor->SetActorRotation(LerpedRotation);
		}
	}
}

void UAblTurnToTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const
{
	Super::OnTaskEnd(Context, result);

	UAblTurnToTaskScratchPad* ScratchPad = Cast<UAblTurnToTaskScratchPad>(Context->GetScratchPadForTask(this));
	check(ScratchPad);

	for (const FTurnToTaskEntry& Entry : ScratchPad->InProgressTurn)
	{
		if (Entry.Actor.IsValid())
		{
#if !(UE_BUILD_SHIPPING)
			if (IsVerbose())
			{
				PrintVerbose(FString::Printf(TEXT("Setting Actor %s rotation to %s ."), *Entry.Actor->GetName(), *Entry.Target.ToCompactString()));
			}
#endif
			Entry.Actor->SetActorRotation(Entry.Target);
		}
	}
}

UAblAbilityTaskScratchPad* UAblTurnToTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return NewObject<UAblTurnToTaskScratchPad>(Context.Get());
}

TStatId UAblTurnToTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblTurnToTask, STATGROUP_Able);
}

FRotator UAblTurnToTask::GetTargetRotation(const AActor* Source, const AActor* Destination) const
{
	check(Source && Destination);
	
	float Yaw = 0.0f;
	float Pitch = 0.0f;

	FVector ToTarget = Destination->GetActorLocation() - Source->GetActorLocation();
	ToTarget.Normalize();

	FVector2D YawPitch = ToTarget.UnitCartesianToSpherical();

	if (m_SetYaw)
	{
		Yaw = FMath::RadiansToDegrees(YawPitch.Y);
	}

	if (m_SetPitch)
	{
		Pitch = FMath::RadiansToDegrees(YawPitch.X);
	}

	FRotator OutRotator(Pitch, Yaw, 0.0f);

	return OutRotator + m_RotationOffset;
}

#if WITH_EDITOR

FText UAblTurnToTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblTurnToTaskFormat", "{0}: {1}");
	FString TargetName = FAbleLogHelper::GetTargetTargetEnumAsString(m_RotationTarget);
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(TargetName));
}

void UAblTurnToTask::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	if (GetDuration() < m_Blend.GetBlendTime())
	{
		m_EndTime = GetStartTime() + m_Blend.GetBlendTime();
	}
}

#endif

#undef LOCTEXT_NAMESPACE