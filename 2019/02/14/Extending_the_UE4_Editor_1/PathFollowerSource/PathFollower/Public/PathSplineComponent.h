// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Math/InterpCurvePoint.h"
#include "PathSplineComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PATHFOLLOWER_API UPathSplineComponent : public USplineComponent
{
	GENERATED_BODY()	
	
public:
	UPathSplineComponent();

	UPathSplineComponent(const FObjectInitializer& ObjectInitializer);

	virtual void Serialize(FArchive& Ar) override;

	void SyncPointInfo();

	virtual FActorComponentInstanceData* GetComponentInstanceData() const override;

	void ApplyComponentInstanceData(class FPathSplineInstanceData* ComponentInstanceData, const bool bPostUCS);

	UFUNCTION(BlueprintCallable, Category = Spline)
	float GetSpeedByIndex(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = Spline)
	float GetWaitTimeByIndex(int32 Index) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spline)
	TArray<FVector> Positions;			//ӳ�䵽SplineCurves.Position.Points�е�����, ��Ҫ��������ƥ��.

	UPROPERTY(EditAnywhere, EditFixedSize, Category = Spline)
	TArray<float> Speeds;				//�����洢��ÿ������ƶ��ٶ�.

	UPROPERTY(EditAnywhere, EditFixedSize, Category = Spline)
	TArray<float> WaitTimes;			//�����洢��ÿ����ĵȴ�ʱ��.

private:
	float DefaultSpeed;

	float DefaultWaitTime;
};
