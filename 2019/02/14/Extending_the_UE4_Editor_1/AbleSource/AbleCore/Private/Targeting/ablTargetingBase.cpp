// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Targeting/ablTargetingBase.h"

#include "AbleCorePrivate.h"
#include "Camera/CameraActor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Targeting/ablTargetingFilters.h"

#define LOCTEXT_NAMESPACE "AblAbilityTargeting"

UAblTargetingBase::UAblTargetingBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	m_AutoCalculateRange(true),
	m_Range(0.0f),
	m_CalculateAs2DRange(true),
	m_Location(),
	m_CollisionChannel(ECollisionChannel::ECC_Pawn),
	m_UseAsync(false)
{

}

UAblTargetingBase::~UAblTargetingBase()
{

}

void UAblTargetingBase::FilterTargets(UAblAbilityContext& Context) const
{
	for (UAblAbilityTargetingFilter* TargetFilter : m_Filters)
	{
		TargetFilter->Filter(Context, *this);
	}
}

FAblAbilityTargetTypeLocation::FAblAbilityTargetTypeLocation()
	: m_Source(EAblAbilityTargetType::Self),
	m_Offset(ForceInitToZero),
	m_Rotation(ForceInitToZero),
	m_Socket(NAME_None),
	m_UseSocketRotation(false)
{

}

void FAblAbilityTargetTypeLocation::GetTargetTransform(const UAblAbilityContext& Context, int32 TargetIndex, FTransform& OutTransform) const
{
	check(m_Source.GetValue() == EAblAbilityTargetType::TargetActor);
	check(TargetIndex < Context.GetTargetActorsWeakPtr().Num());

	TWeakObjectPtr<AActor> TargetActor = Context.GetTargetActorsWeakPtr()[TargetIndex];

	if (TargetActor.IsValid())
	{
		if (!m_Socket.IsNone())
		{
			if (USkeletalMeshComponent* SkeletalMesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (m_UseSocketRotation)
				{
					OutTransform = SkeletalMesh->GetSocketTransform(m_Socket, RTS_Component);
				}
				else
				{
					OutTransform.SetTranslation(SkeletalMesh->GetSocketLocation(m_Socket));
				}
			}
		}
		else
		{
			OutTransform = TargetActor->GetActorTransform();
		}

		OutTransform *= FTransform(m_Rotation);

		FQuat Rotator = OutTransform.GetRotation();

		FVector LocalSpaceOffset = Rotator.GetForwardVector() * m_Offset.X;
		LocalSpaceOffset += Rotator.GetRightVector() * m_Offset.Y;
		LocalSpaceOffset += Rotator.GetUpVector() * m_Offset.Z;

		OutTransform.AddToTranslation(LocalSpaceOffset);
	}
	else
	{
		UE_LOG(LogAble, Warning, TEXT("Unable to find an Actor for our Targeting source, Query will be from the origin. Is this intended?"));

		OutTransform = FTransform(m_Rotation, m_Offset);
	}
}

void FAblAbilityTargetTypeLocation::GetTransform(const UAblAbilityContext& Context, FTransform& OutTransform) const
{
	OutTransform = FTransform::Identity;

	AActor* BaseActor = GetSourceActor(Context);

	if (!BaseActor)
	{
		UE_LOG(LogAble, Warning, TEXT("Unable to find an Actor for our Targeting source, Query will be from the origin. Is this intended?"));

		OutTransform = FTransform(m_Rotation, m_Offset);
	}
	else
	{
		if (!m_Socket.IsNone())
		{
			if (USkeletalMeshComponent* SkeletalMesh = BaseActor->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (m_UseSocketRotation)
				{			
					OutTransform = SkeletalMesh->GetSocketTransform(m_Socket, RTS_Component);
				}
				else
				{
					OutTransform.SetTranslation(SkeletalMesh->GetSocketLocation(m_Socket));
				}
			}
		}
		else
		{
			// If we're not a camera actor, we get our "Eyes point" which is just the camera by another name.
			// Otherwise, it's safe just to grab our actor location.
			if (m_Source == EAblAbilityTargetType::Camera && !BaseActor->IsA<ACameraActor>())
			{
				FVector ActorEyes;
				FRotator ActorEyesRot;
				BaseActor->GetActorEyesViewPoint(ActorEyes, ActorEyesRot);
				OutTransform = FTransform(FQuat(ActorEyesRot), ActorEyes);
			}
			else
			{
				OutTransform = BaseActor->GetActorTransform();
			}
		}

		OutTransform *= FTransform(m_Rotation);

		FQuat Rotator = OutTransform.GetRotation();
		
		FVector LocalSpaceOffset = Rotator.GetForwardVector() * m_Offset.X;
		LocalSpaceOffset += Rotator.GetRightVector() * m_Offset.Y;
		LocalSpaceOffset += Rotator.GetUpVector() * m_Offset.Z;

		OutTransform.AddToTranslation(LocalSpaceOffset);
	}
}

AActor* FAblAbilityTargetTypeLocation::GetSourceActor(const UAblAbilityContext& Context) const
{
	switch (m_Source)
	{
		case EAblAbilityTargetType::Self:
		case EAblAbilityTargetType::Camera: // Camera we just return self.
		{
			return Context.GetSelfActor();
		}
		break;
		case EAblAbilityTargetType::Instigator:
		{
			return Context.GetInstigator();
		}
		break;
		case EAblAbilityTargetType::Owner:
		{
			return Context.GetOwner();
		}
		break;
		case EAblAbilityTargetType::TargetActor:
		{
			check(Context.GetTargetActorsWeakPtr().Num() && Context.GetTargetActorsWeakPtr()[0].IsValid());
			return Context.GetTargetActorsWeakPtr()[0].Get();
		}
		break;
		default:
		{
			checkNoEntry();
		}
		break;
	}

	return nullptr;
}

#if WITH_EDITOR

void UAblTargetingBase::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	if (m_AutoCalculateRange)
	{
		m_Range = CalculateRange();
	}
}

#endif

#undef LOCTEXT_NAMESPACE
