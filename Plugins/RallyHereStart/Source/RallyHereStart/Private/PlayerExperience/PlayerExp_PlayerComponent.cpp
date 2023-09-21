// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "PlayerExperience/PlayerExperienceGlobals.h"
#include "PlayerExperience/PlayerExp_PlayerComponent.h"
#include "PlayerExperience/PlayerExp_PlayerTracker.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/NetConnection.h"

UPlayerExp_PlayerComponent::UPlayerExp_PlayerComponent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectInitializer)
{
    SetIsReplicatedByDefault(true);

    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.SetTickFunctionEnable(true);

    PingStats = FPlayerExp_StatAccumulator();
    FrameTimeStats = FPlayerExp_StatAccumulator();
    FrameTimeStats_Period = FPlayerExp_StatAccumulator();
    NumHitchesDetected = 0;
    NumHitchesDetected_Period = 0;
    NumServerCorrections = 0;

    StatUpdateTime = 0.0;

    OwningPlayerTracker = nullptr;

    UpdateToServerPeriod = 10.0f;
    TimeUntilNextAccumulation = UpdateToServerPeriod;
    LastTime = -1.0;

    bRecordNetCorrections = true;
    LastNetCorrectionTimeStamp = -1.0f;

    CachedPCOwner = nullptr;

    RequiredNumSamplesForHitchDetection = 1000;
    HitchDetectionThreshold = 2.0;
}

void UPlayerExp_PlayerComponent::OnRegister()
{
    StatUpdateTime = FPlatformTime::Seconds();
    ResetTimeUntilNextAccumulation();

    CachedPCOwner = Cast<APlayerController>(GetOwner());

    Super::OnRegister();
}

void UPlayerExp_PlayerComponent::OnUnregister()
{
    CachedPCOwner = nullptr;

    Super::OnUnregister();
}

void UPlayerExp_PlayerComponent::SetPlayerTracker(UPlayerExp_PlayerTracker* InPlayerTracker)
{
    OwningPlayerTracker = InPlayerTracker;
}

void UPlayerExp_PlayerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CachedPCOwner != nullptr)
    {
        check(CachedPCOwner->GetLocalRole() != ROLE_SimulatedProxy);
        const bool bIsLocalPlayer = CachedPCOwner->IsLocalPlayerController();
        bool bIsNewSecond = false;

        double CurrentRealTimeSeconds = FPlatformTime::Seconds();
        if(CurrentRealTimeSeconds - StatUpdateTime > 1.0f)
        {
            bIsNewSecond = true;
            StatUpdateTime = CurrentRealTimeSeconds;
        }

        if (bIsLocalPlayer)
        {
            CollectPerFrameClientStats(DeltaTime);
            if (bIsNewSecond)
            {
                CollectPerSecondClientStats();
            }

            TimeUntilNextAccumulation -= DeltaTime;
            if (TimeUntilNextAccumulation <= 0.0f)
            {
                ResetTimeUntilNextAccumulation();

                CallSendStatsToServer();
            }
        }
        else
        {
            CollectPerFrameServerStats(DeltaTime);
            if (bIsNewSecond)
            {
                CollectPerSecondServerStats();
            }
        }
    }
}

void UPlayerExp_PlayerComponent::CollectPerFrameClientStats(float DeltaTime)
{
    double CalculatedFrameTime = UPlayerExperienceGlobals::CalcFrameTime(LastTime);

    if (CalculatedFrameTime > 0.0)
    {
        FrameTimeStats_Period.AddSample(CalculatedFrameTime);

        //Rudimentary Hitch Detection Algorithm. Checks to see if the current frame time is several standard deviations above the current mean.
        if (FrameTimeStats.GetSampleCount() > RequiredNumSamplesForHitchDetection)
        {
            double FrameDiffFromMean = CalculatedFrameTime - FrameTimeStats.GetMean();
            if (FrameDiffFromMean > 0.0 && (FrameDiffFromMean * FrameDiffFromMean) > (FrameTimeStats.GetVariance() * HitchDetectionThreshold * HitchDetectionThreshold))
            {
                ++NumHitchesDetected_Period;
            }
        }
    }
}

void UPlayerExp_PlayerComponent::CollectPerSecondClientStats()
{

}

void UPlayerExp_PlayerComponent::CallSendStatsToServer()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        ServerFrameTimeUpdate(FrameTimeStats_Period, NumHitchesDetected_Period);
    }

    FrameTimeStats += FrameTimeStats_Period;
    FrameTimeStats_Period.ResetStatistics();
    NumHitchesDetected += NumHitchesDetected_Period;
    NumHitchesDetected_Period = 0;
}

void UPlayerExp_PlayerComponent::CollectPerFrameServerStats(float DeltaTime)
{
    if (bRecordNetCorrections)
    {
        // This is not the most accurate way of recording net corrections since it can only record 1 per tick,
        // but that should be good enough for gauging player experience.
        APawn* RemotePawn = CachedPCOwner->GetPawnOrSpectator();
        if (RemotePawn && (RemotePawn->GetRemoteRole() == ROLE_AutonomousProxy) && !IsNetMode(NM_Client))
        {
            UCharacterMovementComponent* CharacterMovementComponent = Cast<UCharacterMovementComponent>(RemotePawn->GetMovementComponent());
            if (CharacterMovementComponent != nullptr && CharacterMovementComponent->HasValidData())
            {
                FNetworkPredictionData_Server_Character* ServerData = CharacterMovementComponent->GetPredictionData_Server_Character();
                check(ServerData);

                if (ServerData->PendingAdjustment.TimeStamp > 0.0f
                    && !ServerData->PendingAdjustment.bAckGoodMove
                    && LastNetCorrectionTimeStamp < ServerData->PendingAdjustment.TimeStamp)
                {
                    LastNetCorrectionTimeStamp = ServerData->PendingAdjustment.TimeStamp;
                    ++NumServerCorrections;
                }
            }
        }
    }
}

void UPlayerExp_PlayerComponent::CollectPerSecondServerStats()
{
    check(CachedPCOwner != nullptr);

    if (UNetConnection* pNetConnection = CachedPCOwner->NetConnection)
    {
        if(pNetConnection->AvgLag > 0.0f)
        {
            HandleUpdatePing(pNetConnection->AvgLag);
        }
        // These values are copied (not accumulated).
        InTotalPackets = pNetConnection->InTotalPackets;
        InTotalPacketsLost = pNetConnection->InTotalPacketsLost;
        OutTotalPackets = pNetConnection->OutTotalPackets;
        OutTotalPacketsLost = pNetConnection->OutTotalPacketsLost;
    }
}

void UPlayerExp_PlayerComponent::HandleUpdatePing(float InPing)
{
    PingStats.AddSample(InPing * 1000.0f);
}

void UPlayerExp_PlayerComponent::ResetTimeUntilNextAccumulation()
{
    TimeUntilNextAccumulation = FMath::Max(UpdateToServerPeriod + FMath::RandRange(-0.5f, 0.5f), 1.0f);
}

void UPlayerExp_PlayerComponent::ServerFrameTimeUpdate_Implementation(const FPlayerExp_StatAccumulator& InFrameTimeStats, int32 InHitchesDetected)
{
    FrameTimeStats += InFrameTimeStats;
    NumHitchesDetected += InHitchesDetected;
}

bool UPlayerExp_PlayerComponent::ServerFrameTimeUpdate_Validate(const FPlayerExp_StatAccumulator& InFrameTimeStats, int32 InHitchesDetected)
{
    return true;
}
