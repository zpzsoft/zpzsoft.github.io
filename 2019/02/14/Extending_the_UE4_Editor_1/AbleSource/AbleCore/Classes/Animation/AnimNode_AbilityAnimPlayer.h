// Copyright (c) 2016 - 2017 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "AlphaBlend.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Templates/SharedPointer.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "AnimNode_AbilityAnimPlayer.generated.h"

class UAnimSequence;

/* A Special Animation State Node that allows the Play Animation Task to feed in Animations at runtime, it extends the AssetPlayer node.*/
USTRUCT()
struct ABLECORE_API FAnimNode_AbilityAnimPlayer : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()

	FAnimNode_AbilityAnimPlayer();

public:

	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime();
	virtual float GetCurrentAssetLength();
	// End of FAnimNode_AssetPlayerBase interface

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	/* Plays the specified Animation Sequence.*/
	void PlayAnimationSequence(const UAnimSequence* Animation, float PlayRate, const FAlphaBlend& Blend);
	
	/* Special logic to play when interrupts occur.*/
	void OnAbilityInterrupted();

	/* Returns true if our Sequence has been set. It'll be reset once the clip is completed. */
	bool HasAnimationToPlay() const { return m_SequenceToPlay != nullptr; }

	/* Allows the direct setting of the Internal Time value. */
	void SetAnimationTime(float NewTime) { InternalTimeAccumulator = NewTime; }

private:
	/* Helper Method to Reset Internal Time Accumulators.*/
	void ResetInternalTimeAccumulator();

	/* Swap from our Current Sequence to the Next (if it exists). */
	void SwapToNextSequence();

	/* The Sequence we'll be playing. */
	const UAnimSequence* m_SequenceToPlay;

	/* Our play rate (supports negative). */
	float m_PlayRate;

	/* If we're already playing a sequence, this is the next one to queue up. */
	const UAnimSequence* m_NextSequence;

	/* Next Sequence Tick Marker */
	FMarkerTickRecord m_NextSequenceMarkerTickRecord;

	/* Next Sequence Time Accumulator */
	float m_NextSequenceInternalTimeAccumulator;

	/* Next Sequence Blend In */
	FAlphaBlend m_NextSequenceBlendIn;

	/* Next Sequence play rate */
	float m_NextPlayRate;

	/* This is the last valid sequence we played, used in blending if we have any updates after we've reset our sequence to avoid any T-Posing. */
	const UAnimSequence* m_CachedOutputSequence;
};
