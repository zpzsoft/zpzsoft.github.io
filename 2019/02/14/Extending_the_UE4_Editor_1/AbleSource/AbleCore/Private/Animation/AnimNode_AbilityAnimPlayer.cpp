// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#include "Animation/AnimNode_AbilityAnimPlayer.h"

#include "AbleCorePrivate.h"

#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimSequence.h"

FAnimNode_AbilityAnimPlayer::FAnimNode_AbilityAnimPlayer()
	: m_SequenceToPlay(nullptr),
	m_PlayRate(1.0f),
	m_NextSequence(nullptr),
	m_NextSequenceMarkerTickRecord(),
	m_NextSequenceInternalTimeAccumulator(0.0f),
	m_NextSequenceBlendIn(),
	m_NextPlayRate(1.0f),
	m_CachedOutputSequence(nullptr)
{

}

float FAnimNode_AbilityAnimPlayer::GetCurrentAssetTime()
{
	return m_SequenceToPlay ? m_SequenceToPlay->SequenceLength : 0.0f;
}

float FAnimNode_AbilityAnimPlayer::GetCurrentAssetLength()
{
	return InternalTimeAccumulator;
}

void FAnimNode_AbilityAnimPlayer::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	EvaluateGraphExposedInputs.Execute(Context);
}

void FAnimNode_AbilityAnimPlayer::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	EvaluateGraphExposedInputs.Execute(Context);
	if (m_SequenceToPlay && m_SequenceToPlay->SequenceLength - InternalTimeAccumulator < KINDA_SMALL_NUMBER)
	{
		if (m_NextSequence)
		{
			SwapToNextSequence();
		}
		else
		{
			m_SequenceToPlay = nullptr;
		}
	}

	if (m_SequenceToPlay && Context.AnimInstanceProxy->IsSkeletonCompatible(m_SequenceToPlay->GetSkeleton()))
	{
		if (m_NextSequence && m_NextSequenceBlendIn.IsComplete())
		{
			SwapToNextSequence();
		}

		InternalTimeAccumulator = FMath::Clamp(InternalTimeAccumulator, 0.f, m_SequenceToPlay->SequenceLength);

		if (m_NextSequence)
		{
			m_NextSequenceBlendIn.Update(Context.GetDeltaTime());
			m_NextSequenceInternalTimeAccumulator += Context.GetDeltaTime();
			m_NextSequenceInternalTimeAccumulator = FMath::Clamp(m_NextSequenceInternalTimeAccumulator, 0.0f, m_NextSequence->SequenceLength);
		}

		if (FAnimInstanceProxy* Proxy = Context.AnimInstanceProxy)
		{
			FAnimGroupInstance* SyncGroup;
			FAnimTickRecord& SequenceTickRecord = Proxy->CreateUninitializedTickRecord(NAME_None, SyncGroup);
			float CurrentBlendValue = m_NextSequenceBlendIn.GetBlendedValue();
			float SequenceBlendRate = m_NextSequence ? 1.0f - CurrentBlendValue : 1.0f;
			Proxy->MakeSequenceTickRecord(SequenceTickRecord, const_cast<UAnimSequence*>(m_SequenceToPlay), false, m_PlayRate, SequenceBlendRate, InternalTimeAccumulator, MarkerTickRecord);

			if (m_NextSequence)
			{
				FAnimTickRecord& NextSequenceTickRecord = Proxy->CreateUninitializedTickRecord(NAME_None, SyncGroup);
				Proxy->MakeSequenceTickRecord(NextSequenceTickRecord, const_cast<UAnimSequence*>(m_NextSequence), false, m_NextPlayRate, CurrentBlendValue, m_NextSequenceInternalTimeAccumulator, m_NextSequenceMarkerTickRecord);
			}
		}
		//CreateTickRecordForNode(Context, const_cast<UAnimSequence*>(m_SequenceToPlay), false, m_PlayRate);
	}
	else if (m_CachedOutputSequence && Context.AnimInstanceProxy->IsSkeletonCompatible(m_CachedOutputSequence->GetSkeleton()))
	{
		InternalTimeAccumulator = FMath::Clamp(InternalTimeAccumulator, 0.0f, m_CachedOutputSequence->SequenceLength);
		if (FAnimInstanceProxy* Proxy = Context.AnimInstanceProxy)
		{
			FAnimGroupInstance* SyncGroup;
			FAnimTickRecord& SequenceTickRecord = Proxy->CreateUninitializedTickRecord(NAME_None, SyncGroup);
			Proxy->MakeSequenceTickRecord(SequenceTickRecord, const_cast<UAnimSequence*>(m_CachedOutputSequence), false, m_PlayRate, 1.0, InternalTimeAccumulator, MarkerTickRecord);
		}
	}
}

void FAnimNode_AbilityAnimPlayer::Evaluate_AnyThread(FPoseContext& Output)
{
	check(Output.AnimInstanceProxy);
	FAnimInstanceProxy* Proxy = Output.AnimInstanceProxy;
	if (m_SequenceToPlay && Proxy->IsSkeletonCompatible(m_SequenceToPlay->GetSkeleton()))
	{
		if (m_NextSequence && Proxy->IsSkeletonCompatible(m_NextSequence->GetSkeleton()))
		{
			FCompactPose Poses[2];
			FBlendedCurve Curves[2];
			float Weights[2];

			const FBoneContainer& RequiredBone = Proxy->GetRequiredBones();
			Poses[0].SetBoneContainer(&RequiredBone);
			Poses[1].SetBoneContainer(&RequiredBone);

			Curves[0].InitFrom(RequiredBone);
			Curves[1].InitFrom(RequiredBone);

			Weights[0] = 1.0f - m_NextSequenceBlendIn.GetBlendedValue();
			Weights[1] = m_NextSequenceBlendIn.GetBlendedValue();

			m_SequenceToPlay->GetAnimationPose(Poses[0], Curves[0], FAnimExtractContext(InternalTimeAccumulator, Proxy->ShouldExtractRootMotion()));
			m_NextSequence->GetAnimationPose(Poses[1], Curves[1], FAnimExtractContext(m_NextSequenceInternalTimeAccumulator, Proxy->ShouldExtractRootMotion()));

			FAnimationRuntime::BlendPosesTogether(Poses, Curves, Weights, Output.Pose, Output.Curve);
		}
		else
		{
			m_SequenceToPlay->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(InternalTimeAccumulator, Output.AnimInstanceProxy->ShouldExtractRootMotion()));
		}
		m_CachedOutputSequence = m_SequenceToPlay;
	}
	else if (m_CachedOutputSequence)
	{
		m_CachedOutputSequence->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(InternalTimeAccumulator, Output.AnimInstanceProxy->ShouldExtractRootMotion()));
	}
	else
	{
		Output.ResetToRefPose();
	}
}

void FAnimNode_AbilityAnimPlayer::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine, true);
}

void FAnimNode_AbilityAnimPlayer::PlayAnimationSequence(const UAnimSequence* Animation, float PlayRate,const FAlphaBlend& Blend)
{
	check(Animation);

	if (!m_SequenceToPlay)
	{
		m_SequenceToPlay = Animation;
		m_PlayRate = PlayRate;
		ResetInternalTimeAccumulator();
		MarkerTickRecord.Reset();
	}
	else
	{
		if (m_NextSequence)
		{
			SwapToNextSequence();
		}

		m_NextSequence = Animation;
		m_NextPlayRate = PlayRate;
		m_NextSequenceBlendIn = Blend;
		
		if (m_NextSequence && (m_PlayRate * m_NextSequence->RateScale) < 0.0f)
		{
			m_NextSequenceInternalTimeAccumulator = m_NextSequence->SequenceLength;
		}
		else
		{
			m_NextSequenceInternalTimeAccumulator = 0.0f;
		}
		
		m_NextSequenceMarkerTickRecord.Reset();
	}
}

void FAnimNode_AbilityAnimPlayer::OnAbilityInterrupted()
{
	if (m_NextSequence)
	{
		SwapToNextSequence();
	}

	ResetInternalTimeAccumulator();
}

void FAnimNode_AbilityAnimPlayer::ResetInternalTimeAccumulator()
{
	if (m_SequenceToPlay && (m_PlayRate * m_SequenceToPlay->RateScale) < 0.0f)
	{
		InternalTimeAccumulator = m_SequenceToPlay->SequenceLength;
	}
	else
	{
		InternalTimeAccumulator = 0.0f;
	}
}

void FAnimNode_AbilityAnimPlayer::SwapToNextSequence()
{
	m_SequenceToPlay = m_NextSequence;
	m_PlayRate = m_NextPlayRate;
	InternalTimeAccumulator = m_NextSequenceInternalTimeAccumulator;


	m_NextSequence = nullptr;
	m_NextSequenceInternalTimeAccumulator = 0.0f;
	m_NextPlayRate = 0.0f;
	m_NextSequenceBlendIn.Reset();
	m_NextSequenceMarkerTickRecord.Reset();
}
