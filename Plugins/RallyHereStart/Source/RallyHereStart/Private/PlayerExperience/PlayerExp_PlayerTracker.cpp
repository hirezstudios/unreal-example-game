// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "PlayerExperience/PlayerExp_PlayerTracker.h"
#include "PlayerExperience/PlayerExp_MatchTracker.h"
#include "Engine/GameInstance.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "IPAddress.h"

UPlayerExp_PlayerTracker::UPlayerExp_PlayerTracker(const FObjectInitializer& ObjectIntializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectIntializer)
    , PlayerController(nullptr)
{
    PlayerController = nullptr;
    TimeConnected = 0.0;
    LastConnectedTimestamp = -1.0;
    Connections = 0;
    Disconnections = 0;
    ConnectionTimeouts = 0;
    InTotalPackets = 0;
    InTotalPacketsLost = 0;
    OutTotalPackets = 0;
    OutTotalPacketsLost = 0;
    PingStats = FPlayerExp_StatAccumulator();
    FrameTimeStats = FPlayerExp_StatAccumulator();
    NumHitchesDetected = 0;
    NumServerCorrections = 0;

    PlayerComponentClass = nullptr;
    ActivePlayerComponent = nullptr;
}

void UPlayerExp_PlayerTracker::Init()
{
}

void UPlayerExp_PlayerTracker::HandleNetworkConnectionTimeout()
{
    ConnectionTimeouts++;
}

void UPlayerExp_PlayerTracker::HandleMatchEnded()
{
    RecordActiveData(true);
}

void UPlayerExp_PlayerTracker::BindToController(APlayerController* InPlayerController, bool bDoNotRecordConnection /*= false*/)
{
    check(InPlayerController != nullptr);
    APlayerController* pExistingController = GetController();
    if (pExistingController != InPlayerController)
    {
        if (pExistingController != nullptr)
        {
            Unbind(bDoNotRecordConnection);
        }

        PlayerController = InPlayerController;

        if(PlayerController.IsValid())
        {
            if (UPlayerExp_MatchTracker* pOwningMatchTracker = GetMatchTracker())
            {
                pOwningMatchTracker->AddPlayerTrackerToControllerMap(this, PlayerController.Get());
            }

            TSubclassOf<UPlayerExp_PlayerComponent> ComponentClassToUse = PlayerComponentClass;
            if (ComponentClassToUse == nullptr)
            {
                ComponentClassToUse = UPlayerExp_PlayerComponent::StaticClass();
            }

            check(ComponentClassToUse != nullptr);
            check(!ActivePlayerComponent.IsValid());

            if(UPlayerExp_PlayerComponent* pPlayerComponent = NewObject<UPlayerExp_PlayerComponent>(PlayerController.Get(), ComponentClassToUse))
            {
                ActivePlayerComponent = pPlayerComponent;
                pPlayerComponent->SetPlayerTracker(this);
                pPlayerComponent->RegisterComponent();
            }

            if (UNetConnection* pNetConnection = PlayerController->GetNetConnection())
            {
                OnlinePlatformName = pNetConnection->GetPlayerOnlinePlatformName();
                LastConnectedTimestamp = pNetConnection->GetConnectTime();
                if (TSharedPtr<const FInternetAddr> RemoteAddr = pNetConnection->GetRemoteAddr())
                {
                    if (RemoteAddr->GetProtocolType() == FNetworkProtocolTypes::IPv4)
                    {
                        ipv4_address = RemoteAddr->ToString(false);
                        ipv6_address.Empty();
                    }
                    if (RemoteAddr->GetProtocolType() == FNetworkProtocolTypes::IPv6)
                    {
                        ipv4_address.Empty();
                        ipv6_address = RemoteAddr->ToString(false);
                    }
                }
            }
        }

        if(!bDoNotRecordConnection)
        {
            Connections++;
        }
    }
}

void UPlayerExp_PlayerTracker::ControllerLogout(AController* InController)
{
    if (InController != nullptr && InController == GetController())
    {
        Unbind();
    }
}

UPlayerExp_MatchTracker* UPlayerExp_PlayerTracker::GetMatchTracker() const
{
    return Cast<UPlayerExp_MatchTracker>(GetOuter());
}

void UPlayerExp_PlayerTracker::Unbind(bool bDoNotRecordDisconnect /*= false*/)
{
    check(PlayerController.IsValid());

    if (!bDoNotRecordDisconnect)
    {
        Disconnections++;
    }

    RecordActiveData();

    LastConnectedTimestamp = -1.0;

    if (ActivePlayerComponent.IsValid())
    {
        ActivePlayerComponent->DestroyComponent();
    }
    ActivePlayerComponent.Reset();

    if (UPlayerExp_MatchTracker* pOwningMatchTracker = GetMatchTracker())
    {
        pOwningMatchTracker->RemovePlayerTrackerFromControllerMap(PlayerController.Get(), this);
    }

    PlayerController.Reset();
}

void UPlayerExp_PlayerTracker::RecordActiveData(bool bForce /*= false*/)
{
    UPlayerExp_MatchTracker* pMatchTracker = GetMatchTracker();
    if (!bForce && (pMatchTracker == nullptr || !pMatchTracker->IsMatchInProgress()))
    {
        return;
    }

    //TODO: this requires engine modifications to use.
    //APlayerController* pPC = PlayerController.Get();
    //if (pPC != nullptr && pPC->NetConnection != nullptr)
    //{
    //    RecordNetConnectionData(pPC->NetConnection);
    //}
    //else if (UNetConnection::GNetConnectionBeingCleanedUp != nullptr && UNetConnection::GNetConnectionBeingCleanedUp->OwningActor == pPC)
    //{
    //    RecordNetConnectionData(UNetConnection::GNetConnectionBeingCleanedUp);
    //}
    if(PlayerController.IsValid() && LastConnectedTimestamp >= 0.0)
    {
        if (UNetDriver* pNetDriver = PlayerController->GetNetDriver())
        {
            double ElapsedTime = pNetDriver->GetElapsedTime();
            TimeConnected += FMath::Max(0.0, ElapsedTime - LastConnectedTimestamp);
            LastConnectedTimestamp = ElapsedTime;
        }
    }

    if (ActivePlayerComponent.IsValid())
    {
        RecordPlayerComponentData(ActivePlayerComponent.Get());
    }
}

void UPlayerExp_PlayerTracker::RecordNetConnectionData(UNetConnection* InNetConnection)
{
    check(InNetConnection != nullptr);

    InTotalPackets += InNetConnection->InTotalPackets;
    InTotalPacketsLost += InNetConnection->InTotalPacketsLost;
    OutTotalPackets += InNetConnection->OutTotalPackets;
    OutTotalPacketsLost += InNetConnection->OutTotalPacketsLost;

    double ConnectionLifeTime = InNetConnection->Driver != nullptr
        ? InNetConnection->Driver->GetElapsedTime() - InNetConnection->GetConnectTime()
        : 0.0f;

    TimeConnected += ConnectionLifeTime;
}

FORCEINLINE void AddStats(FPlayerExp_StatAccumulator& DestStats, const FPlayerExp_StatAccumulator& ToAddStats)
{
    if (DestStats.GetSampleCount() > 0)
    {
        DestStats += ToAddStats;
    }
    else
    {
        DestStats = ToAddStats;
    }
}

void UPlayerExp_PlayerTracker::RecordPlayerComponentData(UPlayerExp_PlayerComponent* InPlayerComponent)
{
    check(InPlayerComponent != nullptr);

    PingStats += InPlayerComponent->PingStats;
    FrameTimeStats += InPlayerComponent->FrameTimeStats;
    NumHitchesDetected += InPlayerComponent->NumHitchesDetected;
    NumServerCorrections += InPlayerComponent->NumServerCorrections;

    InTotalPackets += InPlayerComponent->InTotalPackets;
    InTotalPacketsLost += InPlayerComponent->InTotalPacketsLost;
    OutTotalPackets += InPlayerComponent->OutTotalPackets;
    OutTotalPacketsLost += InPlayerComponent->OutTotalPacketsLost;
}


void UPlayerExp_PlayerTracker::SetPlayerUuid(const FGuid& InPlayerUuid)
{
	check(InPlayerUuid.IsValid());
	ensureMsgf(!PlayerUuid.IsValid(), TEXT("Player Uuid of a player tracker is being set more than once!"));

	PlayerUuid = InPlayerUuid;
	GetMatchTracker()->AddPlayerTrackerToPlayerUuidMap(this);
}