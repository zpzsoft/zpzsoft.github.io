// Fill out your copyright notice in the Description page of Project Settings.

#include "PathSplineComponent.h"

class FPathSplineInstanceData : public FSceneComponentInstanceData
{
public:
	explicit FPathSplineInstanceData(const UPathSplineComponent* SourceComponent)
		: FSceneComponentInstanceData(SourceComponent)
	{
	}

	virtual void ApplyToComponent(UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override
	{
		FSceneComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);
		CastChecked<UPathSplineComponent>(Component)->ApplyComponentInstanceData(this, (CacheApplyPhase == ECacheApplyPhase::PostUserConstructionScript));
	}

	bool bSplineHasBeenEdited;
	FSplineCurves SplineCurves;
	FSplineCurves SplineCurvesPreUCS;
	TArray<FVector> Positions;
	TArray<float> Speeds;
	TArray<float> WaitTimes;	
};

UPathSplineComponent::UPathSplineComponent()
{
	
}

UPathSplineComponent::UPathSplineComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer), 
	DefaultSpeed(0.2f),
	DefaultWaitTime(0.2f)
{
}

void UPathSplineComponent::Serialize(FArchive & Ar)
{
	SyncPointInfo();
	return Super::Serialize(Ar);
}

void UPathSplineComponent::ApplyComponentInstanceData(FPathSplineInstanceData* SplineInstanceData, const bool bPostUCS)
{
	check(SplineInstanceData);

	if (bPostUCS)
	{
		if (bInputSplinePointsToConstructionScript)
			return;
		else
		{
			bModifiedByConstructionScript = (SplineInstanceData->SplineCurvesPreUCS != SplineCurves);

			TArray<UProperty*> Properties;
			Properties.Emplace(FindField<UProperty>(USplineComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves)));
			RemoveUCSModifiedProperties(Properties);
		}
	}
	else
		SplineInstanceData->SplineCurvesPreUCS = SplineCurves;

	if (SplineInstanceData->bSplineHasBeenEdited)
	{
		SplineCurves = SplineInstanceData->SplineCurves;
		bModifiedByConstructionScript = false;
	}

	Positions = SplineInstanceData->Positions;
	Speeds = SplineInstanceData->Speeds;
	WaitTimes = SplineInstanceData->WaitTimes;
	bSplineHasBeenEdited = SplineInstanceData->bSplineHasBeenEdited;

	UpdateSpline();
}

float UPathSplineComponent::GetSpeedByIndex(int32 Index) const
{
	if (Index < 0 || Index >= Speeds.Num())
		return -1;

	return Speeds[Index];
}

float UPathSplineComponent::GetWaitTimeByIndex(int32 Index) const
{
	if (Index < 0 || Index >= Speeds.Num())
		return -1;

	return WaitTimes[Index];
}

FActorComponentInstanceData * UPathSplineComponent::GetComponentInstanceData() const
{
	FPathSplineInstanceData* SplineInstanceData = new FPathSplineInstanceData(this);
	if (Super::bSplineHasBeenEdited)
		SplineInstanceData->SplineCurves = Super::SplineCurves;

	SplineInstanceData->Positions = Positions;
	SplineInstanceData->Speeds = Speeds;
	SplineInstanceData->WaitTimes = WaitTimes;
	SplineInstanceData->bSplineHasBeenEdited = Super::bSplineHasBeenEdited;

	return SplineInstanceData;
}

void UPathSplineComponent::SyncPointInfo()
{
	int pointCnt = SplineCurves.Position.Points.Num();
	int diffCnt = abs(pointCnt - Positions.Num());

	switch (diffCnt)
	{
		case 0:
		{
			//数量一致, 实时同步最新的point坐标.
			for(int i = 0; i < pointCnt; i++)
				Positions[i] = SplineCurves.Position.Points[i].OutVal;

			break;
		}
		case 1:
		{
			//找出在哪个点有增减, 然后在该索引操作对应的数组.
			int diffIndex = -1;
			int curCnt = FMath::Min(pointCnt, Positions.Num());

			for(int i = 0; i < curCnt; i++)
			{
				if(Positions[i] != SplineCurves.Position.Points[i].OutVal)
				{
					diffIndex = i;
					break;
				}
			}

			if (diffIndex == -1) diffIndex = curCnt;

			if(pointCnt < Positions.Num())
			{
				Positions.RemoveAt(diffIndex);
				Speeds.RemoveAt(diffIndex);
				WaitTimes.RemoveAt(diffIndex);
			}
			else
			{
				Positions.Insert(SplineCurves.Position.Points[diffIndex].OutVal, diffIndex);
				Speeds.Insert(DefaultSpeed, diffIndex);
				WaitTimes.Insert(DefaultWaitTime, diffIndex);
			}

			break;
		}
		default:
		{
			//差异太多, 直接清空之后同步.
			Positions.Empty();
			Speeds.Empty();
			WaitTimes.Empty();
		
			for(int i = 0; i  < pointCnt; i++)
			{
				Positions.Add(SplineCurves.Position.Points[i].OutVal);
				Speeds.Add(DefaultSpeed);
				WaitTimes.Add(DefaultWaitTime);
			}
		}
	}
}
