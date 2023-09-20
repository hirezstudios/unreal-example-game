// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "PlayerExperience/PlayerExperienceGlobals.h"
#include "PlayerExperience/PlayerExp_MatchTracker.h"
#include "PlayerExperience/PlayerExp_MatchReportSender.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogPlayerExperience);

int64 UPlayerExperienceGlobals::JsonConverterSkipFlags = CPF_Deprecated | CPF_Transient | CPF_Config;

UPlayerExperienceGlobals::UPlayerExperienceGlobals(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PlayerExperienceGlobalsClassName = FSoftClassPath(TEXT("/Script/PlayerExperience.PlayerExperienceGlobals"));
}

void UPlayerExperienceGlobals::InitGlobalData(UGameInstance* InGameInstance)
{
    if (!Enabled)
    {
        return;
    }

    if (InGameInstance == nullptr)
    {
        return;
    }

    if (InGameInstance->IsDedicatedServerInstance())
    {
        CreateNewMatchTracker(InGameInstance);
    }
}

void UPlayerExperienceGlobals::UninitGlobalData(UGameInstance* InGameInstance)
{
    if (InGameInstance == nullptr)
    {
        return;
    }

    if (ActiveMatchTracker != nullptr && ActiveMatchTracker->GetOwningGameInstance() == InGameInstance)
    {
        ActiveMatchTracker->DoCleanup();
        ActiveMatchTracker = nullptr;
    }
}

UPlayerExp_MatchTracker* UPlayerExperienceGlobals::CreateNewMatchTracker(UGameInstance* InGameInstance)
{
    if (!Enabled)
    {
        return nullptr;
    }

    if (InGameInstance == nullptr)
    {
        return nullptr;
    }

    // This must either be a dedicated server, standalone game, or listen server.
    if (!InGameInstance->IsDedicatedServerInstance() && (InGameInstance->GetWorld() == nullptr || InGameInstance->GetWorld()->GetNetMode() == NM_Client))
    {
        return nullptr;
    }

    if (ActiveMatchTracker != nullptr)
    {
        if (ensure(ActiveMatchTracker->GetOwningGameInstance() == InGameInstance))
        {
            ActiveMatchTracker->DoCleanup();
        }
        ActiveMatchTracker = nullptr;
    }

    if (MatchTrackerClassName.IsValid())
    {
        UClass* MatchTrackerClass = LoadClass<UObject>(NULL, *(MatchTrackerClassName.ToString()), NULL, LOAD_None, NULL);
        if (MatchTrackerClass)
        {
            ActiveMatchTracker = NewObject<UPlayerExp_MatchTracker>(this, MatchTrackerClass, NAME_None);
        }
    }

    if (ActiveMatchTracker == nullptr)
    {
        // Fallback to base native class
        ActiveMatchTracker = NewObject<UPlayerExp_MatchTracker>(this, UPlayerExp_MatchTracker::StaticClass(), NAME_None);
    }

    check(ActiveMatchTracker != nullptr);
    ActiveMatchTracker->DoInit(InGameInstance);
    return ActiveMatchTracker;
}

UPlayerExp_MatchTracker* UPlayerExperienceGlobals::GetMatchTracker(UWorld* InWorld) const
{
    return InWorld != nullptr ? GetMatchTracker(InWorld->GetGameInstance()) : nullptr;
}

UPlayerExp_MatchTracker* UPlayerExperienceGlobals::GetMatchTracker(UGameInstance* InGameInstance) const
{
    return InGameInstance != nullptr && ActiveMatchTracker != nullptr && ActiveMatchTracker->GetOwningGameInstance() == InGameInstance
        ? ActiveMatchTracker
        : nullptr;
}

double UPlayerExperienceGlobals::CalcFrameTime(double& InLastTime)
{
    double CalculatedFrameTime = 0.0;

    // [11/4/2020 rfredericksen] - experiment 
    if (FApp::IsBenchmarking() || FApp::UseFixedTimeStep())
    {
        /** If we're in fixed time step mode, FApp::GetCurrentTime() will be incorrect for benchmarking */
        if (InLastTime >= 0.0)
        {
            const double CurrentTime = FPlatformTime::Seconds() * 1000.0;
            CalculatedFrameTime = CurrentTime - InLastTime;
            InLastTime = CurrentTime;

        }
        else
        {
            InLastTime = FPlatformTime::Seconds() * 1000.0;
        }
    }
    else
    {
        /** Use the DiffTime we computed last frame, because it correctly handles the end of frame idling and corresponds better to the other unit times. */
        CalculatedFrameTime = (FApp::GetCurrentTime() - FApp::GetLastTime()) * 1000.0;
    }

    return CalculatedFrameTime;
}