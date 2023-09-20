// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/ObjectKey.h"
#include "Tickable.h"
#include "PlayerExperience/PlayerExp_StatAccumulator.h"
#include "Engine/EngineBaseTypes.h"
#include "PlayerExperience/PlayerExp_PlayerTracker.h"
#include "Misc/DateTime.h"
#include "PlayerExp_MatchTracker.generated.h"

class UGameInstance;
class FJsonObject;
class FJsonValue;
class FMatchFrameTracker;
class AGameModeBase;
class UWorld;
class UNetDriver;
class UNetConnection;
class AController;
class APlayerController;

UCLASS(Config=Game)
class RALLYHERESTART_API UPlayerExp_MatchTracker : public UObject
{
    GENERATED_BODY()

public:
    UPlayerExp_MatchTracker(const FObjectInitializer& ObjectIntializer = FObjectInitializer::Get());
    virtual void BeginDestroy() override;

	virtual UPlayerExp_PlayerTracker* CreateOrGetPlayerTrackerForController(APlayerController* InPlayerController);
	virtual UPlayerExp_PlayerTracker* CreateOrGetPlayerTrackerFromPlayerUuid(const FGuid& InPlayerUuid);
	UPlayerExp_PlayerTracker* GetPlayerTrackerForPlayerUuid(const FGuid& InPlayerUuid) const;
	UPlayerExp_PlayerTracker* GetPlayerTrackerForController(APlayerController* InPlayerController) const;

	virtual void OnPlayerAdded(const FGuid& InUuid);

    void DoInit(UGameInstance* InOwningGameInstance);
    void DoCleanup();

    virtual void Tick(float DeltaTime);

	void RegisterSessionCallbacks();
	void UnregisterSessionCallbacks();

	void BindAllAddedPlayers();

    TSharedPtr<FJsonObject> CreateReport();
    TSharedPtr<FJsonObject> CacheMatchReport();
    void SendMatchReport(bool bForceRecache = false);

    void DoStartMatch();
    void DoEndMatch(float MatchScoreDeviation);
    void MarkMatchFubar(const FString& InFubarReason);

    FORCEINLINE bool IsInitialized() const { return bInitialized; }
    FORCEINLINE bool IsMatchInProgress() const { return bMatchInProgress; }

    UGameInstance* GetOwningGameInstance() const { return OwningGameInstance.Get(); }
    AGameModeBase* GetActiveGameMode() const { return ActiveGameMode.Get(); }

	UPROPERTY(Config)
	TSubclassOf<UPlayerExp_PlayerTracker> PlayerTrackerClass;

protected:
    virtual void Init();
    virtual void Cleanup();
    virtual void StartMatch();
    virtual void EndMatch(float MatchScoreDeviation);
    virtual void GameModeSwitched();
    virtual TSharedPtr<FJsonValue> CreateReport_MatchInfo();

	class URH_JoinedSession* GetSession() const;
	void OnSessionUpdated(class URH_SessionView* Session);

    //============================================================================
    // THIS VERSION NUMBER MUST BE UPDATED WHEN THE JSON SCHEMA CHANGES.
    // That means all name changes, deletions, or additions of fields of any sort.
    static const int32 SchemaVersionNumber;
    //============================================================================

    UPROPERTY()
    FString HostName;

	UPROPERTY()
	FString SessionId;

	UPROPERTY()
	FString InstanceId;

	UPROPERTY()
	FString AllocationId;

	UPROPERTY()
	FString Region;

    UPROPERTY(Transient)
    TWeakObjectPtr<UGameInstance> OwningGameInstance;

    UPROPERTY()
    TArray<UPlayerExp_PlayerTracker*> Players;
    TMap<FObjectKey, TWeakObjectPtr<UPlayerExp_PlayerTracker>> PlayerTrackerByControllerMap;

    UPROPERTY(Transient)
    bool bInitialized;

    UPROPERTY(Transient)
    UClass* LoadedPlayerTrackerClass;

    UPROPERTY(Transient)
    bool bMatchInProgress;

    UPROPERTY(Transient)
    double MatchStartTimeSeconds;
    UPROPERTY(Transient)
    double MatchEndTime;
    UPROPERTY()
    double MatchDuration;
    UPROPERTY()
    FDateTime MatchStartTime;

    // Automatically Call DoStartMatch() when a game mode is initialized.
    UPROPERTY(Config)
    bool bAutoStartMatch;
    // Immediately Send a report when the match ends.
    UPROPERTY(Config)
    bool bAutoSendReportOnMatchEnded;

    // The match has been marked "Fubar" (Something went wrong).
    UPROPERTY()
    bool MatchIsFubar;

    UPROPERTY()
    FString FubarReason;

    UPROPERTY()
    FPlayerExp_StatAccumulator FrameTimeStats;

    UPROPERTY(Transient)
    double LastTime;

    // A Report Cached inside EndMatch()
    TSharedPtr<FJsonObject> CachedMatchReport;

	UPROPERTY()
	class UPlayerExp_MatchReportSender* MatchReportSender;

	TMap<FGuid, TWeakObjectPtr<UPlayerExp_PlayerTracker>> PlayerTrackerByPlayerUuid;

    void CaptureStatsForReport();

    UPlayerExp_PlayerTracker* CreateNewPlayerTracker();
    void AddPlayerTrackerToControllerMap(UPlayerExp_PlayerTracker* InTracker, APlayerController* InController);
    bool RemovePlayerTrackerFromControllerMap(AController* InController, const UPlayerExp_PlayerTracker* InExpectedTracker);
    void SwapPlayerTrackerOnControllerMap(UPlayerExp_PlayerTracker* InTracker, APlayerController* InOldPC, APlayerController* InNewPC);

	void AddPlayerTrackerToPlayerUuidMap(class UPlayerExp_PlayerTracker* InTracker);

    friend class UPlayerExp_PlayerTracker;

    virtual void BindAllControllers();

protected:
    virtual void GameModeInitialized(AGameModeBase* InGameMode);
    virtual void GameModePostLogin(AGameModeBase* InGameMode, APlayerController* InPlayerController);
    virtual void GameModeLogout(AGameModeBase* InGameMode, AController* InController);

    UPROPERTY(Transient)
    TWeakObjectPtr<AGameModeBase> ActiveGameMode;

    TUniquePtr<FMatchFrameTracker> FrameTrackerPtr;

    FDelegateHandle GameModeInitializedHandle;
    FDelegateHandle GameModePostLoginHandle;
    FDelegateHandle GameModeLogoutHandle;
    FDelegateHandle NetworkFailureHandle;

	friend class FMatchFrameTracker;
};

class FMatchFrameTracker : public FTickableGameObject
{
public:
    FMatchFrameTracker(UPlayerExp_MatchTracker* InOwner);
    ~FMatchFrameTracker();

    void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }
    virtual void Tick( float DeltaTime ) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const;

private:
    bool bEnabled;
    TWeakObjectPtr<UPlayerExp_MatchTracker> OwningMatchTracker;
};