// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "GameFramework/RHPlayerInput.h"
#include "Interfaces/OnlineGameMatchesInterface.h"
#include "Inventory/RHLoadoutTypes.h"
#include "RHPlayerController.generated.h"

class ARHPlayerController;
class ARHPlayerState;

UENUM(BlueprintType)
enum class ESonyMatchState : uint8
{
	NotStarted,
	MatchIdRequested,
	Playing,
	SendPauseOrCancelMatch,
	SendCompleteMatch,
	Complete,
};

UCLASS(Config=Game)
class RALLYHERESTART_API ARHPlayerController : public APlayerController
{
    GENERATED_UCLASS_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetRHPlayerUuid(FGuid InRHPlayerUuid) { RHPlayerUuid = InRHPlayerUuid; }
	FGuid GetRHPlayerUuid() const { return RHPlayerUuid; }

	/** Return the client to the main menu gracefully */
	virtual void ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason) override;

    virtual void SetInputBinds();

    void PushToTalkPressed();
    void PushToTalkReleased();

	UFUNCTION(BlueprintCallable, Category="UIX")
	void UIX_FlushPressedKeys() { FlushPressedKeys(); };
	
	// Use WWise Motion for Force Feedback
	void SetUseForceFeedback(bool bUseForceFeedback);

	class URHPlayerInput* GetRHPlayerInput() const;

protected:
	/** Allows the PlayerController to set up custom input bindings. */
    virtual void SetupInputComponent() override;

	UPROPERTY(Replicated)
	FGuid RHPlayerUuid;

    UPROPERTY(EditAnywhere, Category = "Input")
    TSubclassOf<class UInputComponent> InputComponentClass;

private:
	bool WasForceFeedbackEnabled;

public:
	 // BEGIN - PS5 Match Handling
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(Reliable, Client)
    void ClientCheckSonyMatchOwnerEligibility();

    UFUNCTION(Reliable, Client)
    void ClientUpdateSonyMatchData(const FString& InMatchId, const FString& InActivityId);

    UFUNCTION(Reliable, Server)
    void ServerUpdateSonyMatchData(const FString& InMatchId);

    UFUNCTION(Reliable, Server)
    void ServerUpdateSonyMatchOwnerEligibility(bool bIsEligible);

    virtual void RequestUpdateSonyMatchState(ESonyMatchState NewSonyMatchState);

    void UpdateSonyMatchResult(const FFinalGameMatchReport& InSonyMatchResult, bool bShouldCompleteMatch);

    bool IsEligibleSonyMatchOwner() const { return bIsEligibleSonyMatchOwner; }

    static bool PlatformSupportsSonyMatchApi();

    static FString GetSonyTeamIdFromActorTeamNum(int32 ActorTeamNum);

    static FString GetSonyTeamDisplayNameFromActorTeamNum(int32 ActorTeamNum);

protected:
    void UpdateSonyMatchRoster();

    FGameMatchRoster SonyMatchRoster;

    FFinalGameMatchReport SonyMatchResult;

    void SetSonyMatchId(const FString& InSonyMatchId);

    UPROPERTY()
    FString SonyMatchId;

    UPROPERTY()
    FString SonyActivityId;

    UPROPERTY()
    ESonyMatchState SonyMatchState;

    UPROPERTY()
    ESonyMatchState QueuedSonyMatchState;

    UPROPERTY()
    bool bIsSonyMatchOwner;

    UPROPERTY()
    bool bIsEligibleSonyMatchOwner;

    UPROPERTY()
    bool bIsExclusiveSonyMatchOwner;

    TArray<TSharedRef<const FUniqueNetId>> SonyCrossPlayPlayerIds;
    // END - PS5 Match Handling
};
