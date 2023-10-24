// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Lobby/Widgets/RHQueueWidgetBase.h"
#include "RHQuickPlayWidget.generated.h"

// Queue state in the context of this display widget
UENUM(BlueprintType)
enum class EQuickPlayQueueState : uint8
{
	Unknown                                 UMETA(DisplayName = "Unknown"),
	NoQueuesAvailable                       UMETA(DisplayName = "No Queues Available"),
	NoSelectedQueue                         UMETA(DisplayName = "No Queue Selected"),
	SelectedQueueUnavailable                UMETA(DisplayName = "Selected Queue Unavailable"),
	SelectedQueuePartyMinLimit              UMETA(DisplayName = "Selected Queue Party Minimum Limit"),
	SelectedQueuePartyMaxLimit              UMETA(DisplayName = "Selected Queue Party Maximum Limit"),
	ReadyToJoin                             UMETA(DisplayName = "Ready to Join Queue"),
	WaitingForLeader                        UMETA(DisplayName = "Waiting For Party Leader"),
	Queued                                  UMETA(DisplayName = "Queued"),
	EnteringMatch                           UMETA(DisplayName = "Entering Match"),
	ReadyToRejoin                           UMETA(DisplayName = "Ready to Rejoin Match"),
	PlayerLevelRequirement                  UMETA(DisplayName = "Player Does Not Meet Level Requirement"),
	PartyLevelRequirement                   UMETA(DisplayName = "A Party Member Does Not Meet Level Requirement"),
	PartyRankRequirement                    UMETA(DisplayName = "Ranked Level Range too far in Party"),
	PartyPlatformRequirement                UMETA(DisplayName = "Selected Queue Party Platform Requirement"),
	PlayerOwnedJobRequirement				UMETA(DisplayName = "Player Owns Not Enough Rogues"),
	PartyOwnedJobRequirement				UMETA(DisplayName = "Party Member Owns Not Enough Rogues"),
};

/**
 * Quick Play Button display widget on home screen
 */
UCLASS()
class RALLYHERESTART_API URHQuickPlayWidget : public URHQueueWidgetBase
{
	GENERATED_UCLASS_BODY()

	virtual void							InitializeWidget_Implementation() override;
	virtual void ShowWidget(ESlateVisibility InVisibility = ESlateVisibility::SelfHitTestInvisible) override; //$$ LDP - Added visibility param

	virtual void                            UpdateQueueSelection();

	/**
	* State change update overloads
	**/
protected:
	virtual void                            OnQueuePermissionUpdate_Implementation(bool CanQueue) override;
	virtual void                            OnControlQueuePermissionUpdate_Implementation(bool CanControl) override;
	virtual void                            OnSelectedQueueUpdate_Implementation(URH_MatchmakingQueueInfo* CurrentSelectedQueue) override;
	virtual void                            OnQueueStateUpdate_Implementation(ERH_MatchStatus CurrentMatchStatus) override;

	UFUNCTION(BlueprintCallable, Category = "Quick Play Widget")
	void                                    UpdateState();
	EQuickPlayQueueState                    DetermineQuickPlayState() const;

	UFUNCTION(BlueprintPure, Category = "Quick Play Widget")
	EQuickPlayQueueState                    GetSelectedQueueState() const;

private: 
	EQuickPlayQueueState                    CurrentQuickPlayState;

	/**
	* Display getters
	**/
protected:
	// return the current local-context display queue state
	UFUNCTION(BlueprintPure, Category = "Quick Play Widget")
	FORCEINLINE EQuickPlayQueueState       GetCurrentQuickPlayState() const { return CurrentQuickPlayState; }
		
	// Get the display name for the currently selected queue's gamemode; returns false when no valid queue is currently selected
	UFUNCTION(BlueprintPure, Category = "Quick Play Widget")
	bool                                    GetGameModeDisplayName(FText& GameModeDisplayName);

	/**
	* Display change events
	**/
	// signals an update to the quick play display state
	UFUNCTION(BlueprintImplementableEvent, Category = "Quick Play Widget")
	void                                    OnUpdateQuickPlayState(EQuickPlayQueueState QueueState);

	// signals an update to the quick play interactability
	UFUNCTION(BlueprintImplementableEvent, Category = "Quick Play Widget")
	void                                    OnUpdateQuickPlayCanPlay(bool CanPlay);

	// update penalty time left
	UFUNCTION(BlueprintImplementableEvent, Category = "Quick Play Widget")
	void                                    OnUpdatePenaltyTimeLeft(int32 TimeLeft = 0);
	// count-up for time elapsed in queue
	UFUNCTION(BlueprintImplementableEvent, Category = "Quick Play Widget")
	void                                    OnUpdateQueueTimeElapsed(float TimeElapsed = 0.f);
private:
	void HandleElapsedTimer();

public:
	UFUNCTION(BlueprintPure)
	bool IsPendingQueueUpdate() { return bPendingQueueUpdate; };
	UFUNCTION(BlueprintCallable)
	void SetIsPendingQueueUpdate(bool IsPending) { bPendingQueueUpdate = IsPending; };

private:
	// UI-level flag that helps us identify when we've asked the server to do something (enter / leave queue) but have not received an update yet.
	// This lets us give feedback immediately even if the server has a delay. It also lets us block spamming requests since we know we've already sent one.
	bool bPendingQueueUpdate;

	FTimerHandle ElapsedTimer;
	FTimerHandle PenaltyTimer;
};
