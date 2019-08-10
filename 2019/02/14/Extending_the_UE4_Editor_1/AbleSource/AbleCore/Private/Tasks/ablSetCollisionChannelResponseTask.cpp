// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Tasks/ablSetCollisionChannelResponseTask.h"

#include "AbleCorePrivate.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "Misc/EnumRange.h"
#include "Stats/Stats2.h"

#if !(UE_BUILD_SHIPPING)
#include "ablAbilityUtilities.h"
#endif

ENUM_RANGE_BY_FIRST_AND_LAST(ECollisionChannel, ECC_WorldStatic, ECC_MAX);

#define LOCTEXT_NAMESPACE "AblAbilityTask"

UAblSetCollisionChannelResponseTaskScratchPad::UAblSetCollisionChannelResponseTaskScratchPad()
{

}

UAblSetCollisionChannelResponseTaskScratchPad::~UAblSetCollisionChannelResponseTaskScratchPad()
{

}

UAblSetCollisionChannelResponseTask::UAblSetCollisionChannelResponseTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_Channel(ECollisionChannel::ECC_WorldDynamic),
	m_Response(ECollisionResponse::ECR_Ignore),
	m_SetAllChannelsToResponse(false),
	m_RestoreOnEnd(true)
{

}

UAblSetCollisionChannelResponseTask::~UAblSetCollisionChannelResponseTask()
{

}

void UAblSetCollisionChannelResponseTask::OnTaskStart(const TWeakObjectPtr<const UAblAbilityContext>& Context) const
{
	Super::OnTaskStart(Context);

	UAblSetCollisionChannelResponseTaskScratchPad* ScratchPad = nullptr;
	
	if (m_RestoreOnEnd)
	{
		ScratchPad = Cast<UAblSetCollisionChannelResponseTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);
	}

	// We need to convert our Actors to primitive components.
	TArray<TWeakObjectPtr<AActor>> TargetArray;
	GetActorsForTask(Context, TargetArray);

	TArray<TWeakObjectPtr<UPrimitiveComponent>> PrimitiveComponents;

	for (TWeakObjectPtr<AActor>& Target : TargetArray)
	{
		if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
		{
			PrimitiveComponents.AddUnique(PrimitiveComponent);
		}
	}

	for (TWeakObjectPtr<UPrimitiveComponent>& Component : PrimitiveComponents)
	{
		if (Component.IsValid())
		{
			if (m_RestoreOnEnd)
			{
				if (m_SetAllChannelsToResponse)
				{
					const FCollisionResponseContainer& Container = Component->GetCollisionResponseToChannels();
					for (ECollisionChannel Channel : TEnumRange<ECollisionChannel>())
					{
						ScratchPad->PreviousCollisionValues.Add(FCollisionLayerResponseEntry(Component.Get(), Channel, Container.GetResponse(Channel)));
					}
				}
				else
				{
					ScratchPad->PreviousCollisionValues.Add(FCollisionLayerResponseEntry(Component.Get(), m_Channel.GetValue(), Component->GetCollisionResponseToChannel(m_Channel)));
				}
			}

			if (m_SetAllChannelsToResponse)
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Setting All Collision Responses on Actor %s to %s."), *Component->GetOwner()->GetName(), *FAbleLogHelper::GetCollisionResponseEnumAsString(m_Response.GetValue())));
				}
#endif
				Component->SetCollisionResponseToAllChannels(m_Response.GetValue());
			}
			else
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Setting Collision Channel %s Response on Actor %s to %s."), *FAbleLogHelper::GetCollisionChannelEnumAsString(m_Channel.GetValue()), *Component->GetOwner()->GetName(), *FAbleLogHelper::GetCollisionResponseEnumAsString(m_Response.GetValue())));
				}
#endif
				Component->SetCollisionResponseToChannel(m_Channel.GetValue(), m_Response.GetValue());
			}
		}
	}
}

void UAblSetCollisionChannelResponseTask::OnTaskEnd(const TWeakObjectPtr<const UAblAbilityContext>& Context, const EAblAbilityTaskResult result) const
{
	Super::OnTaskEnd(Context, result);

	if (m_RestoreOnEnd)
	{
		UAblSetCollisionChannelResponseTaskScratchPad* ScratchPad = Cast<UAblSetCollisionChannelResponseTaskScratchPad>(Context->GetScratchPadForTask(this));
		check(ScratchPad);

		for (const FCollisionLayerResponseEntry& Entry : ScratchPad->PreviousCollisionValues)
		{
			if (Entry.Primitive.IsValid())
			{
#if !(UE_BUILD_SHIPPING)
				if (IsVerbose())
				{
					PrintVerbose(FString::Printf(TEXT("Setting Collision Channel %s Response on Actor %s to %s."), *FAbleLogHelper::GetCollisionChannelEnumAsString(Entry.Channel), *Entry.Primitive->GetOwner()->GetName(), *FAbleLogHelper::GetCollisionResponseEnumAsString(Entry.Response)));
				}
#endif
				Entry.Primitive->SetCollisionResponseToChannel(Entry.Channel, Entry.Response);
			}
		}
	}
}

UAblAbilityTaskScratchPad* UAblSetCollisionChannelResponseTask::CreateScratchPad(const TWeakObjectPtr<UAblAbilityContext>& Context) const
{
	return m_RestoreOnEnd ? NewObject<UAblSetCollisionChannelResponseTaskScratchPad>(Context.Get()) : nullptr;
}

TStatId UAblSetCollisionChannelResponseTask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAblSetCollisionChannelResponseTask, STATGROUP_Able);
}

#if WITH_EDITOR

FText UAblSetCollisionChannelResponseTask::GetDescriptiveTaskName() const
{
	const FText FormatText = LOCTEXT("AblSetCollisionChannelResponseTaskFormat", "{0}: {1}->{2}");
	FString CollisionChannelName = m_SetAllChannelsToResponse ? TEXT("All") : FAbleLogHelper::GetCollisionChannelEnumAsString(m_Channel);
	FString ResponseName = FAbleLogHelper::GetCollisionResponseEnumAsString(m_Response);
	return FText::FormatOrdered(FormatText, GetTaskName(), FText::FromString(CollisionChannelName), FText::FromString(ResponseName));
}

#endif

#undef LOCTEXT_NAMESPACE