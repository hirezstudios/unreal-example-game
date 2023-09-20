// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlayerExperience/PlayerExp_StatAccumulator.h"
#include "PlayerExperience/PlayerExp_PlayerComponent.h"
#include "PlayerExp_PlayerTracker.generated.h"

class AController;
class APlayerController;
class UPlayerExp_MatchTracker;
class FJsonValue;

UCLASS(Config=Game)
class RALLYHERESTART_API UPlayerExp_PlayerTracker : public UObject
{
    GENERATED_BODY()

public:
    UPlayerExp_PlayerTracker(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual void Init();
    virtual void HandleNetworkConnectionTimeout();
    virtual void HandleMatchEnded();
    virtual void BindToController(APlayerController* InPlayerController, bool bDoNotRecordConnection = false);
    virtual void ControllerLogout(AController* InController);

	void SetPlayerUuid(const FGuid& InPlayerId);
	FORCEINLINE const FGuid& GetPlayerUuid() const { return PlayerUuid; }

    FORCEINLINE APlayerController* GetController() const { return PlayerController.Get(); }
    UPlayerExp_MatchTracker* GetMatchTracker() const;
    void CaptureStatsForReport()
    {
        PingStats.CaptureStatsForReport();
        FrameTimeStats.CaptureStatsForReport();
    }

protected:
    virtual void Unbind(bool bDoNotRecordDisconnect = false);
    virtual void RecordActiveData(bool bForce = false);
    virtual void RecordNetConnectionData(UNetConnection* InNetConnection);
    virtual void RecordPlayerComponentData(UPlayerExp_PlayerComponent* InPlayerComponent);

    UPROPERTY(Transient)
    TWeakObjectPtr<APlayerController> PlayerController;

    // The amount of time this player is connected to the match.
    UPROPERTY()
    double TimeConnected;

    // The time at which this player last connected.
    UPROPERTY(Transient)
    double LastConnectedTimestamp;

    // The last IPv4 Address this player connected from.
    UPROPERTY()
    FString ipv4_address;

    // The last IPv6 Address this player connected from.
    UPROPERTY()
    FString ipv6_address;

    // Platform Name the player connected from (sent as part of server login).
    UPROPERTY()
    FName OnlinePlatformName;

    // The number of times this player connected to the match.
    UPROPERTY()
    int32 Connections;

    // The number of times this player disconnected from the match.
    UPROPERTY()
    int32 Disconnections;

    // The number of times this player timed out.
    UPROPERTY()
    int32 ConnectionTimeouts;

    // The total number of InPackets received from this player.
    UPROPERTY()
    int32 InTotalPackets;

    // The total number of InPackets lost from this player.
    UPROPERTY()
    int32 InTotalPacketsLost;

    // The total number of OutPackets sent to this player.
    UPROPERTY()
    int32 OutTotalPackets;

    // The total number of OutPackets lost by this player.
    UPROPERTY()
    int32 OutTotalPacketsLost;

    // The player's average ping (and other stats).
    UPROPERTY()
    FPlayerExp_StatAccumulator PingStats;

    // The player's average frame time (and Other stats).
    UPROPERTY()
    FPlayerExp_StatAccumulator FrameTimeStats;

    // The number of times the player experienced a hitch (Frame time is 1.5 times greater than the standard deviation)
    UPROPERTY()
    int32 NumHitchesDetected;

    // The number of times the player experienced a server correction.
    UPROPERTY()
    int32 NumServerCorrections;

    UPROPERTY(Config)
    TSubclassOf<UPlayerExp_PlayerComponent> PlayerComponentClass;

    UPROPERTY(Transient)
    TWeakObjectPtr<UPlayerExp_PlayerComponent> ActivePlayerComponent;

    UPROPERTY()
    FGuid PlayerUuid;
};