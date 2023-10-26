// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "Engine/GameInstance.h"
#include "Engine/MapBuildDataRegistry.h"
#include "Engine/DataTable.h"
#include "GameplayTags.h"
#include "OnlineError.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/IMessageSanitizerInterface.h"
#include "Misc/Guid.h"
#include "RHGameInstance.generated.h"

namespace ERHNetworkError
{
	enum Type : uint8
	{
		NoErrors,
		DisconnectedXboxLive,
		DisconnectedPlaystationNetwork,
		DisconnectedUnknown,
		SignoutXboxLive,
		SignoutPlaystationNetwork,
		SignoutUnknown,
	};
}

class FRHAppSettingHotfixInstance;

UCLASS(config = Game, transient, BlueprintType, Blueprintable)
class RALLYHERESTART_API URHGameInstance : public UGameInstance
{
    GENERATED_UCLASS_BODY()

public:
	virtual void Init() override;
    virtual void Shutdown() override;

    ERHNetworkError::Type RetrieveCurrentNetworkError() const;
	void ClearCurrentNetworkError();

	void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);

    void HandleUserLoginChanged(int32 GameUserIndex, ELoginStatus::Type PreviousLoginStatus, ELoginStatus::Type LoginStatus, const FUniqueNetId& UserId);

	/** Sets a rich presence string for all local players. */
	void SetPresenceForLocalPlayers(const FText& DisplayStatus, const class FVariantData& PresenceData);

	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;

    DECLARE_EVENT(URHGameInstance, FLoadingScreenEndedEvent);
    FLoadingScreenEndedEvent& OnLoadingScreenEnded() { return LoadingScreenEndedEvent; }

    //this is here to ensure persistence
    TMap<FString, FString> NameToURLJsonMapping;

    UPROPERTY(Config)
    bool bLogoffOnAppSuspend;

    UPROPERTY(Config)
    bool bLogoffOnAppResume;

	FOnLocalPlayerEvent OnLocalPlayerLoginChanged;

protected:
    UFUNCTION()
    virtual void BeginLoadingScreen(const FString& MapName);
    UFUNCTION()
    virtual void EndLoadingScreen(class UWorld* World);

    // these involve the application losing focus
    virtual void AppSuspendCallbackInGameThread();
    virtual void AppResumeCallbackInGameThread();

    // these involve the game shutting down and pausing (such as when a console is put to sleep)
    virtual void AppDeactivatedCallbackInGameThread();
    virtual void AppReactivatedCallbackInGameThread();

    virtual void SetupAndroidAffinity();

private:
    FLoadingScreenEndedEvent LoadingScreenEndedEvent;
	ERHNetworkError::Type CurrentNetworkError;

	// Handle application resuming from suspension (console)
    void AppSuspendCallback();
    FDelegateHandle AppSuspendHandle;

	void AppResumeCallback();
	FDelegateHandle AppResumeHandle;

    void AppDeactivatedCallback();
    FDelegateHandle AppDeactivatedHandle;

    void AppReactivatedCallback();
    FDelegateHandle AppReactivatedHandle;

public:
	void HandleCheckPerUserPrivilegeResults(const FBlockedQueryResult& QueryResult, int32 MessageId);

protected:
	FDelegateHandle AppResumeDelegateHandle;
	FDelegateHandle AppReactivatedDelegateHandle;

	void ChangeMuteStatusOfPlayerHelper(bool bShouldMute, int32 PlayerIndex);

	void RefreshMuteStatusesHelper();

public:
	void AddReportedPlayer(const FGuid& PlayerId, const FString& InstanceId) { ReportedPlayers.Add(TPair<FGuid, FString>(PlayerId, InstanceId)); }
	bool HasReportedPlayer(const FGuid& PlayerId, const FString& InstanceId) { return ReportedPlayers.Contains(TPair<FGuid, FString>(PlayerId, InstanceId)); }

	void AddReportedServer(const FString& MatchId) { ReportedServers.Add(MatchId); }
	bool HasReportedServer(const FString& MatchId) { return ReportedServers.Contains(MatchId); }

protected:
	//NOTE: Player Reporting utilizes InstanceId while Server Reporting utilizes MatchId
	TSet<TPair<FGuid, FString>> ReportedPlayers;
	TSet<FString> ReportedServers;

public:
	// Allow for Deferred Initialization of the Challenge Manager in order to detect Mobile Game Modes
	void InitializeChallengeManager();
};
