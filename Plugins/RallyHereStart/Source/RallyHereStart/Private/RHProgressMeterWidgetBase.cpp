// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RHProgressMeterWidgetBase.h"

void URHProgressMeterWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (IsDeltaAnimating)
	{
		CurrentAnimTime = FMath::Min(CurrentAnimTime + InDeltaTime, AnimEndTime);

		ApplyMeterPercentages(AnimBasePercent, (CurrentAnimTime / AnimEndTime) * AnimTargetDeltaPercent);

		if (CurrentAnimTime == AnimEndTime)
		{
			IsDeltaAnimating = false;

            // Based on direction we are animating trigger a level completion event if the bar is full or empty
            bool LevelCompleted = false;

            if (AnimTargetDeltaPercent >= 0 && AnimBasePercent != 1.0f)
            {
                LevelCompleted = (AnimBasePercent + AnimTargetDeltaPercent) >= 1.0f;
            }
            else if (AnimTargetDeltaPercent < 0)
            {
                LevelCompleted = (AnimBasePercent + AnimTargetDeltaPercent) <= 0.0f;
            }

            OnDeltaAnimationFinished(LevelCompleted);
		}
		else
		{
			OnDeltaAnimationTicked();
		}
	}
}

// Sets values needed for the animation such that playing the animation is now just a matter of when to tell it to start.
// This won't start the animation on its own, but it will stop one in progress and pre-set with the first "frame" of the new animation.
void URHProgressMeterWidgetBase::SetDeltaAnimationParams(float BasePercent, float DeltaPercent, float AnimTime)
{
	AnimBasePercent = BasePercent;
	AnimTargetDeltaPercent = DeltaPercent;
	AnimEndTime = AnimTime;

	IsDeltaAnimating = false;
	CurrentAnimTime = 0.f;

	ApplyMeterPercentages(AnimBasePercent, 0.f);
}

void URHProgressMeterWidgetBase::PlayDeltaAnimation(float StartDelay)
{
	if (StartDelay > 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(AnimStartDelayTimer, this, &URHProgressMeterWidgetBase::EnableDeltaAnimation, StartDelay, false);
			return;
		}
	}

	EnableDeltaAnimation();
	return;
}

void URHProgressMeterWidgetBase::EnableDeltaAnimation()
{
	IsDeltaAnimating = true;
	OnDeltaAnimationStarted();
}

void URHProgressMeterWidgetBase::ApplyMeterPercentages_Raw(float BasePercent, float DeltaPercent)
{
	// Ensure that the base value is in bounds
	float ClampedBase = FMath::Clamp(BasePercent, 0.f, 1.f);

	// Ensure that base + delta is in bounds
	float ClampedDelta = FMath::Clamp(DeltaPercent, 0.f - ClampedBase, 1.f - ClampedBase);

	ApplyMeterPercentages(ClampedBase, ClampedDelta);
}