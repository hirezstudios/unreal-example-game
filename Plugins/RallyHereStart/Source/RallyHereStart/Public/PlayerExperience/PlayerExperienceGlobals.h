// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPath.h"
#include "PlayerExperienceGlobals.generated.h"

class UWorld;
class UGameInstance;
class UPlayerExp_MatchTracker;
class UPlayerExp_MatchReportSender;

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerExperience, Log, All);

/** Holds global data for the Platform Data Factories. Can be configured per project via config file */
UCLASS(config=Game)
class RALLYHERESTART_API UPlayerExperienceGlobals : public UObject
{
    GENERATED_BODY()

public:
    UPlayerExperienceGlobals(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /** Gets the single instance of the globals object, will create it as necessary */
    static UPlayerExperienceGlobals& Get()
    {
		static UPlayerExperienceGlobals* PlayerExperienceGlobals = nullptr;

		// Defer loading of globals to the first time it is requested
		if (!PlayerExperienceGlobals)
		{
			PlayerExperienceGlobals = NewObject<UPlayerExperienceGlobals>();
			PlayerExperienceGlobals->AddToRoot();
		}

		check(PlayerExperienceGlobals);
		return *PlayerExperienceGlobals;
    }

    /** Can inside UGameInstance::Init() to perform init setup of the data factories */
    virtual void InitGlobalData(UGameInstance* InGameInstance);

    /** Can inside UGameInstance::Shutdown() to teardown the data factories */
    virtual void UninitGlobalData(UGameInstance* InGameInstance);

public:
    UPlayerExp_MatchTracker* CreateNewMatchTracker(UGameInstance* InGameInstance);
    UPlayerExp_MatchTracker* GetMatchTracker(UWorld* InWorld) const;
    UPlayerExp_MatchTracker* GetMatchTracker(UGameInstance* InGameInstance) const;

    UPROPERTY(GlobalConfig)
    bool Enabled;;

    /** The class to instantiate as the globals object. Defaults to this class but can be overridden */
    UPROPERTY(GlobalConfig)
    FSoftClassPath PlayerExperienceGlobalsClassName;
	
	UPROPERTY(Config)
	FSoftClassPath MatchTrackerClassName;

    UPROPERTY()
    UPlayerExp_MatchTracker* ActiveMatchTracker;

    UPROPERTY(Config)
    FSoftClassPath MatchReportSenderClassName;

    UPROPERTY()
    UPlayerExp_MatchReportSender* MatchReportSender;

	static int64 JsonConverterSkipFlags;

    static double CalcFrameTime(double& InLastTime);
};
