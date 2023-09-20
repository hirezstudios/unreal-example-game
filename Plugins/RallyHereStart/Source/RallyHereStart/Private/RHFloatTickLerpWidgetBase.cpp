// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RHFloatTickLerpWidgetBase.h"

URHFloatTickLerpWidgetBase::URHFloatTickLerpWidgetBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void URHFloatTickLerpWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (LerpProgress < 1.f)
	{
		LerpProgress = FMath::Min(LerpProgress += InDeltaTime / LerpTime, 1.f);
		CurrentValue = FMath::Lerp(StartingValue, TargetValue, FMath::Pow(LerpProgress, LerpPower));
		DisplayForValue(CurrentValue);

		if (LerpProgress >= 1.f)
		{
			OnLerpComplete.Broadcast();
		}
	}
}

void URHFloatTickLerpWidgetBase::SetTargetValue(float Value)
{
	TargetValue = Value;
	LerpProgress = 0;
	StartingValue = CurrentValue;
}

void URHFloatTickLerpWidgetBase::ForceCurrentValue(float Value)
{
	StartingValue = Value;
	CurrentValue = Value;
	TargetValue = Value;
	LerpProgress = 1.f;
}