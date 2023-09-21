// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerExperience/PlayerExp_StatAccumulator.h"
#include "PlayerExp_PlayerComponent.generated.h"

class UPlayerExp_PlayerTracker;
class APlayerController;

UCLASS(Config=Game)
class RALLYHERESTART_API UPlayerExp_PlayerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerExp_PlayerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void OnRegister() override;
    virtual void OnUnregister() override;

    void SetPlayerTracker(UPlayerExp_PlayerTracker* InPlayerTracker);
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    UPROPERTY(Transient)
    FPlayerExp_StatAccumulator FrameTimeStats;

    UPROPERTY(Transient)
    double StatUpdateTime;

    UPROPERTY(Transient)
    FPlayerExp_StatAccumulator PingStats;

    // The number of times the player experienced a hitch (Frame time is 1.5 times greater than the standard deviation)
    UPROPERTY(Transient)
    int32 NumHitchesDetected;

    UPROPERTY(Transient)
    int32 NumServerCorrections;

    // The total number of InPackets received from this player.
    UPROPERTY(Transient)
    int32 InTotalPackets;

    // The total number of InPackets lost from this player.
    UPROPERTY(Transient)
    int32 InTotalPacketsLost;

    // The total number of OutPackets sent to this player.
    UPROPERTY(Transient)
    int32 OutTotalPackets;

    // The total number of OutPackets lost by this player.
    UPROPERTY(Transient)
    int32 OutTotalPacketsLost;

protected:
    virtual void CollectPerFrameClientStats(float DeltaTime);
    virtual void CollectPerSecondClientStats();
    virtual void CallSendStatsToServer();

    virtual void CollectPerFrameServerStats(float DeltaTime);
    virtual void CollectPerSecondServerStats();

    virtual void HandleUpdatePing(float InPing);
    void ResetTimeUntilNextAccumulation();

    UPROPERTY(Transient)
    TWeakObjectPtr<UPlayerExp_PlayerTracker> OwningPlayerTracker;

    UPROPERTY(Transient)
    double LastTime;
    UPROPERTY(Transient)
    FPlayerExp_StatAccumulator FrameTimeStats_Period;

    UPROPERTY(Transient)
    int32 NumHitchesDetected_Period;

    // How long to wait between server updates.
    UPROPERTY(Config)
    float UpdateToServerPeriod;
    UPROPERTY(Transient)
    float TimeUntilNextAccumulation;

    // Should this component attempt to record net corrections.
    UPROPERTY(Config)
    bool bRecordNetCorrections;
    UPROPERTY(Transient)
    float LastNetCorrectionTimeStamp;

    /******* Hitch detection parameters *******/
    //How many samples to collect before we have enough data to detect frame hitches on the client.
    UPROPERTY(Config)
    int32 RequiredNumSamplesForHitchDetection;

    //How many standard deviations above the mean frame time must be to considered a hitch.
    UPROPERTY(Config)
    double HitchDetectionThreshold;

    UFUNCTION(unreliable, WithValidation, Server)
    void ServerFrameTimeUpdate(const FPlayerExp_StatAccumulator& InFrameTimeStats, int32 InHitchesDetected);

    UPROPERTY(Transient)
    APlayerController* CachedPCOwner;
};