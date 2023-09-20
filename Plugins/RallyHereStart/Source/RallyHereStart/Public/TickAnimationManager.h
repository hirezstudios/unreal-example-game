// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TickAnimationManager.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FTickAnimationUpdate, float, ElapsedTime, float, ElapsedAlpha);
DECLARE_DYNAMIC_DELEGATE(FTickAnimationFinished);

USTRUCT(BlueprintType)
struct FTickAnimationParams
{
	GENERATED_BODY()

	// Properties
	UPROPERTY(BlueprintReadOnly)
	float Duration;
	UPROPERTY()
	FTickAnimationUpdate UpdateEvent;
	UPROPERTY()
	FTickAnimationFinished FinishedEvent;

	// Runtime variables
	UPROPERTY(BlueprintReadOnly)
	bool IsPlaying;
	UPROPERTY(BlueprintReadOnly)
	float ElapsedTime;

	FTickAnimationParams(float InDuration, const FTickAnimationUpdate& UpdateEventRef, const FTickAnimationFinished& FinishedEventRef)
        : Duration(InDuration)
        , UpdateEvent(UpdateEventRef)
        , FinishedEvent(FinishedEventRef)
        , IsPlaying(false)
        , ElapsedTime(0.f)
    {
    }

	FTickAnimationParams()
        : Duration(0.f)
        , IsPlaying(false)
        , ElapsedTime(0.f)
    {
    }

	~FTickAnimationParams() {}
};

/**
 * 
 */
UCLASS(BlueprintType)
class RALLYHERESTART_API UTickAnimationManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION()
	void ApplyTick(float DeltaTime);

	UFUNCTION()
	void AddAnimation(FName AnimName, float Duration, const FTickAnimationUpdate& UpdateEvent, const FTickAnimationFinished& FinishedEvent);

	UFUNCTION()
	void RemoveAnimation(FName AnimName);

	// Playback controls
	UFUNCTION()
	void PlayAnimation(FName AnimName);
	UFUNCTION()
	void StopAnimation(FName AnimName);
	UFUNCTION()
	void PauseAnimation(FName AnimName);
	UFUNCTION()
	void ResumeAnimation(FName AnimName);
	UFUNCTION()
	void SkipToEndAnimation(FName AnimName);

	UFUNCTION()
	bool GetAnimationInfo(FName AnimName, FTickAnimationParams& OutAnimParams);

protected:
	UPROPERTY()
	TMap<FName, FTickAnimationParams> AnimsByName;
};
