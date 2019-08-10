// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/IAblAbilityTask.h"

#include "ablAbility.h"
#include "ablAbilityComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Targeting/ablTargetingBase.h"

#if !(UE_BUILD_SHIPPING)
#include "ablSettings.h"
#include "ablAbilityUtilities.h"
#if WITH_EDITOR
#include "Editor.h"
#endif
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetSystemLibrary.h"
#endif

UAblAbilityTask::UAblAbilityTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_StartTime(0.0f),
	m_EndTime(1.0f)
{
	
}

UAblAbilityTask::~UAblAbilityTask()
{

}

void UAblAbilityTask::PostInitProperties()
{
	Super::PostInitProperties();

	if (!m_TaskTargets.Num())
	{
		m_TaskTargets.Add(EAblAbilityTargetType::Self);
	}
}

bool UAblAbilityTask::CanStart(const TWeakObjectPtr<const UAblAbilityContext>& Context, float CurrentTime, float DeltaTime) const
{
	return CurrentTime + DeltaTime >= GetStartTime();
}

void UAblAbilityTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
#if !(UE_BUILD_SHIPPING)
	if (IsVerbose())
	{
		PrintVerbose(FString::Printf(TEXT("OnTaskStart called for Task %s at time %2.2f."), *GetName(), Context->GetCurrentTime()));
	}
#endif
}

void UAblAbilityTask::OnTaskTick(const TWeakObjectPtr<const UAblAbilityContext>& Context, float deltaTime) const
{
#if !(UE_BUILD_SHIPPING)
	if (IsVerbose())
	{
		PrintVerbose(FString::Printf(TEXT("OnTaskTick called for Task %s at time %2.2f. Delta Time = %1.5f"), *GetName(), Context->GetCurrentTime(), deltaTime));
	}
#endif
}

bool UAblAbilityTask::IsDone(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	return Context.IsValid() ? Context->GetCurrentTime() >= GetEndTime() : true;
}

void UAblAbilityTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const
{
#if !(UE_BUILD_SHIPPING)
	if (IsVerbose())
	{
		PrintVerbose(FString::Printf(TEXT("OnTaskEnd called for Task %s at time %2.2f. Task Result = %s"), *GetName(), Context->GetCurrentTime(), *FAbleLogHelper::GetTaskResultEnumAsString(result)));
	}
#endif
}

bool UAblAbilityTask::IsValidForNetMode(ENetMode NetMode) const
{
	if (NetMode == NM_Standalone)
	{
		return true;
	}

	switch (GetTaskRealm())
	{
	case EAblAbilityTaskRealm::Client:
	{
		return NetMode == NM_Client || NetMode == NM_ListenServer;
	}
	break;
	case EAblAbilityTaskRealm::Server:
	{
		return NetMode == NM_DedicatedServer || NetMode == NM_ListenServer;
	}
	break;
	case EAblAbilityTaskRealm::ClientAndServer:
	{
		return true;
	}
	break;
	default:
		break;
	}

	return false;
}

#if WITH_EDITOR

float UAblAbilityTask::GetEstimatedTaskCost() const
{
	float EstimatedCost = 0.0f;
	if (!IsSingleFrame())
	{
		EstimatedCost += IsAsyncFriendly() ? ABLTASK_EST_ASYNC : ABLTASK_EST_SYNC;
	}
	EstimatedCost += NeedsTick() ? ABLTASK_EST_TICK : 0.0f;

	return EstimatedCost;
}

void UAblAbilityTask::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	m_StartTime = FMath::Max(0.0f, m_StartTime);
	m_EndTime = FMath::Max(m_EndTime, m_StartTime + 0.001f);

#if WITH_EDITORONLY_DATA
	m_OnTaskPropertyModified.Broadcast(*this, PropertyChangedEvent);
#endif
}

void UAblAbilityTask::AddDependency(const UAblAbilityTask* Task)
{
	m_Dependencies.AddUnique(Task);

#if WITH_EDITORONLY_DATA
	FPropertyChangedEvent ChangeEvent(GetClass()->FindPropertyByName(FName(TEXT("m_Dependencies"))));
	m_OnTaskPropertyModified.Broadcast(*this, ChangeEvent);
#endif
}

void UAblAbilityTask::RemoveDependency(const UAblAbilityTask* Task)
{
	m_Dependencies.Remove(Task);

#if WITH_EDITORONLY_DATA
	FPropertyChangedEvent ChangeEvent(GetClass()->FindPropertyByName(FName(TEXT("m_Dependencies"))));
	m_OnTaskPropertyModified.Broadcast(*this, ChangeEvent);
#endif
}

#endif

void UAblAbilityTask::GetActorsForTask(const TWeakObjectPtr<const UAblAbilityContext>& Context, TArray<TWeakObjectPtr<AActor>>& OutActorArray) const
{
	verify(m_TaskTargets.Num() != 0 && Context.IsValid());

	OutActorArray.Empty();
	for (TEnumAsByte<EAblAbilityTargetType> target : m_TaskTargets)
	{
		switch (target)
		{
			case EAblAbilityTargetType::Camera:
			case EAblAbilityTargetType::Self:
			{
				AActor* SelfActor = Context->GetSelfActor();

				if (IsTaskValidForActor(SelfActor))
				{
					OutActorArray.Add(SelfActor);			
				}
			}
			break;
			case EAblAbilityTargetType::Owner:
			{
				AActor* Owner = Context->GetOwner();
				if (IsTaskValidForActor(Owner))
				{
					OutActorArray.Add(Owner);
				}				
			}
			break;
			case EAblAbilityTargetType::Instigator:
			{
				AActor* Instigator = Context->GetInstigator();
				if (IsTaskValidForActor(Instigator))
				{
					OutActorArray.Add(Instigator);
				}
			}
			break;
			case EAblAbilityTargetType::TargetActor:
			{
				const TArray<TWeakObjectPtr<AActor>>& UnfilteredTargets = Context->GetTargetActorsWeakPtr();
				for (const TWeakObjectPtr<AActor>& Target : UnfilteredTargets)
				{
					if (IsTaskValidForActor(Target.Get()))
					{
						OutActorArray.Add(Target);
					}
				}
			}
			break;
			default:
			{
				checkNoEntry();
			}
			break;
		}
	}
}

AActor* UAblAbilityTask::GetSingleActorFromTargetType(const TWeakObjectPtr<const UAblAbilityContext>& Context, EAblAbilityTargetType TargetType) const
{
	check(Context.IsValid());

	switch (TargetType)
	{
		case EAblAbilityTargetType::Camera:
		case EAblAbilityTargetType::Self:
		{
			if (AActor* Actor = Context->GetSelfActor())
			{
				if (IsTaskValidForActor(Actor))
				{
					return Actor;
				}
			}
			return nullptr;
		}
		break;
		case EAblAbilityTargetType::Instigator:
		{
			if (AActor* Actor = Context->GetInstigator())
			{
				if (IsTaskValidForActor(Actor))
				{
					return Actor;
				}
			}
			return nullptr;
		}
		break;
		case EAblAbilityTargetType::Owner:
		{
			if (AActor* Actor = Context->GetOwner())
			{
				if (IsTaskValidForActor(Actor))
				{
					return Actor;
				}
			}
			return nullptr;
		}
		break;
		case EAblAbilityTargetType::TargetActor:
		{
			if (!Context->GetTargetActors().Num())
			{
				return nullptr;
			}

			if (AActor* Actor = Context->GetTargetActors()[0])
			{
				if (IsTaskValidForActor(Actor))
				{
					return Actor;
				}
			}
			return nullptr;
		}
		break;
		default:
		{
		}
		break;
	}

	return nullptr;
}

bool UAblAbilityTask::IsTaskValidForActor(const AActor* Actor) const
{
	if (!Actor || !Actor->GetWorld())
	{
		return false;
	}
	
	bool ActorLocallyControlled = false;
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		ActorLocallyControlled = Pawn->IsLocallyControlled();
	}

	ENetMode WorldNetMode = Actor->GetWorld()->GetNetMode();
	if (WorldNetMode == ENetMode::NM_Standalone)
	{
		// Standalone / Offline game, no need to worry about networking.
		return true;
	}
	else
	{
		switch (GetTaskRealm())
		{
			case EAblAbilityTaskRealm::Client:
			{
				// Client tasks are safe to run on any proxy/auth/etc.
				return WorldNetMode == NM_Client || ActorLocallyControlled || WorldNetMode == NM_ListenServer;
			}
			break;
			case EAblAbilityTaskRealm::Server:
			{
				return WorldNetMode != NM_Client;
			}
			break;
			case EAblAbilityTaskRealm::ClientAndServer:
			{
				return true;
			}
			break;
			default:
				break;
		}
	}

	return false;
}

#if !(UE_BUILD_SHIPPING)

void UAblAbilityTask::PrintVerbose(const FString& Output) const
{
	static const UAbleSettings* Settings = nullptr;
	if (!Settings)
	{
		Settings = GetDefault<UAbleSettings>();
	}

	UWorld* World = nullptr;

#if WITH_EDITOR
	// The play world needs to handle these commands if it exists
	if (GIsEditor && GEditor->PlayWorld && !GIsPlayInEditorWorld)
	{
		World = GEditor->PlayWorld;
	}
#endif

	ULocalPlayer* Player = GEngine->GetDebugLocalPlayer();
	if (Player)
	{
		UWorld* PlayerWorld = Player->GetWorld();
		if (!World)
		{
			World = PlayerWorld;
		}
	}

#if WITH_EDITOR
	if (!World)
	{
		World = GEditor->GetEditorWorldContext().World();
	}
#endif

	if (World)
	{
		UKismetSystemLibrary::PrintString(World, Output, Settings ? Settings->GetEchoVerboseToScreen() : false, Settings ? Settings->GetLogVerbose() : true, FLinearColor(0.0, 0.66, 1.0), Settings ? Settings->GetVerboseScreenLifetime() : 2.0f);
	}
}

#endif