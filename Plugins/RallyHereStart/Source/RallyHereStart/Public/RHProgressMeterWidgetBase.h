// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHProgressMeterWidgetBase.generated.h"

/**
 * This came from nativizing code in WBP_ProgressEarnedBar, but this base class
 * could be used for other progress meters that show a fill percent and a delta percent that extends (+) or eats into (-) the displayed base percent,
 * and which also can trigger animations of the delta value.
 *
 * This supports the anim parameters being set first, and then the anim starting some time later.
 */
UCLASS()
class RALLYHERESTART_API URHProgressMeterWidgetBase : public URHWidget
{
	GENERATED_BODY()
	
public:
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Apply Meter Percentages"))
	void ApplyMeterPercentages_Raw(float BasePercent, float DeltaPercent);

protected:

	// Set the meter to statically show a fill and a delta affecting that fill.
	UFUNCTION(BlueprintImplementableEvent)
	void ApplyMeterPercentages(float BasePercent, float DeltaPercent);

	// Set the meter to animate its display by using a delta that starts at 0 and goes to DeltaPercent
	UFUNCTION(BlueprintCallable)
	void SetDeltaAnimationParams(float BasePercent, float DeltaPercent, float AnimTime);
	UFUNCTION(BlueprintCallable)
	void PlayDeltaAnimation(float StartDelay);

	UFUNCTION(BlueprintPure)
	bool IsPlayingDeltaAnimation() { return IsDeltaAnimating; };

	UFUNCTION()
	void EnableDeltaAnimation();

	// Callbacks to implement widget-specific responses (e.g. audio, followup logic in the owner) at certain points in the animation
	UFUNCTION(BlueprintImplementableEvent)
	void OnDeltaAnimationStarted();
	UFUNCTION(BlueprintImplementableEvent)
	void OnDeltaAnimationTicked();
	UFUNCTION(BlueprintImplementableEvent)
	void OnDeltaAnimationFinished(bool bLevelChange);

private:
	float AnimBasePercent;
	float AnimTargetDeltaPercent;

	bool IsDeltaAnimating;
	float CurrentAnimTime;
	float AnimEndTime;

	FTimerHandle AnimStartDelayTimer;
};
