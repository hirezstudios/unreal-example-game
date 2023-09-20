// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RHFloatTickLerpWidgetBase.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFloatTickLerpComplete);

UCLASS()
class RALLYHERESTART_API URHFloatTickLerpWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	URHFloatTickLerpWidgetBase(const FObjectInitializer& ObjectInitializer);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Native is handling the number value, but blueprint still needs to define the visual representation.
	UFUNCTION(BlueprintImplementableEvent)
	void DisplayForValue(float Value);

public:

	// Sets the target for the lerp and also resets the lerp to be from wherever it is now to this new value.
	UFUNCTION(BlueprintCallable, Category = "Float Tick Lerp")
	void SetTargetValue(float Value);

	// Gets the current lerping value. It's the same as the target value when the lerp has finished.
	UFUNCTION(BlueprintPure, Category = "Float Tick Lerp")
	float GetCurrentValue() { return CurrentValue; };

	// Set a value without lerping. This will interrupt lerping.
	UFUNCTION(BlueprintCallable, Category = "Float Tick Lerp")
	void ForceCurrentValue(float Value);

	// Returns whether a lerp is in progress.
	UFUNCTION(BlueprintPure, Category = "Float Tick Lerp")
	bool IsLerping() { return LerpProgress != 1; };

	// Sets the time from lerp start to finish. If changed mid-lerp, that lerp will change speed and not restart.
	UFUNCTION(BlueprintCallable, Category = "Float Tick Lerp")
	void SetLerpTime(float Time) { LerpTime = Time; };

	// Sets the positive exponent of the lerp's approach toward the target value. 1 is linear; less than 1 starts fast then eases into the target; greater than 1 starts slow then approaches the target suddenly.
	UFUNCTION(BlueprintCallable, Category = "Float Tick Lerp")
	void SetLerpPower(float Power) { LerpPower = FMath::Max(0.f, Power); };

	// Fires when the lerp completes
	UPROPERTY(BlueprintAssignable, Category = "Float Tick Lerp")
	FOnFloatTickLerpComplete OnLerpComplete;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn))
	float LerpTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn))
	float LerpPower;

	float LerpProgress;

	float StartingValue;
	float TargetValue;

	float CurrentValue;
};
