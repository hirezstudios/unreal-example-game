#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHPartyManager.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "RHContextMenu.generated.h"

UENUM(BlueprintType)
enum class EPlayerContextMenuContext : uint8
{
	Friends UMETA(DisplayName = "Friends"),
	Party UMETA(DisplayName = "Party"),
	CustomLobby UMETA(DisplayName = "Custom Lobby"),
	InGame UMETA(DisplayName = "InGame"),
	Default UMETA(DisplayName = "Default")
};

UENUM(BlueprintType)
enum class EPlayerContextOptions : uint8
{
	AddFriend UMETA(DisplayName = "Add Friend"),
	AddRHFriend UMETA(DisplayName = "Add Rally Here Friend"),
	PartyInvite UMETA(DisplayName = "Party Invite"),
	LobbySwapTeam UMETA(DisplayName = "Lobby Swap Team"),
	LobbyKickPlayer UMETA(DisplayName = "Lobby Kick Player"),
	LobbyPromotePlayer UMETA(DisplayName = "Lobby Promote Player"),
	PartyKick UMETA(DisplayName = "Party Kick"),
	Whisper UMETA(DisplayName = "Whisper"),
	ViewProfile UMETA(DisplayName = "View Profile"),
	ViewPlatformProfile UMETA(DisplayName = "View Platform Profile"),
	RemoveFriend UMETA(DisplayName = "Remove Friend"),
	CancelRequest UMETA(DisplayName = "Cancel Request"),
	AcceptFriendRequest UMETA(DisplayName = "Accept Friend Request"),
	RejectFriendRequest UMETA(DisplayName = "RejectFriendRequest"),
	PromotePartyLeader UMETA(DisplayName = "Promote Party Leader"),
	AcceptPartyInvite UMETA(DisplayName = "Accept Party Invite"),
	DeclinePartyInvite UMETA(DisplayName = "Decline Party Invite"),
	LeaveParty UMETA(DisplayName = "Leave Party"),
	Mute UMETA(DisplayName = "Mute"),
	Unmute UMETA(DisplayName = "Unmute"),
	ReportPlayer UMETA(DisplayName = "Report Player"),
	IgnorePlayer UMETA(DisplayName = "Block Player"),
	UnignorePlayer UMETA(DisplayName = "Unblock Player"),
	None UMETA(DisplayName = "None")
};

UENUM()
enum class EFriendAction : uint8
{
	AddFriend,
	RemoveFriend,
	CancelFriendRequest,
	AcceptFriendRequest,
	RejectFriendRequest
};

UENUM()
enum class EPartyManagerAction : uint8
{
	KickMember,
	PromoteToLeader,
	AcceptInvite,
	DenyInvite,
	LeaveParty
};

UENUM()
enum class EQueueDataFactoryAction : uint8
{
	SwapPlayerCustomMatch,
	KickFromCustomMatch,
	SwitchHostCustomMatch
};

UENUM(BlueprintType)
enum class EViewSide : uint8
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	None UMETA(DisplayName = "None")
};

/*
* Player Context Menu
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnContextOptionsUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReportPlayer, class URH_RHFriendAndPlatformFriend*, ReportedPlayerTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContextOptionCompleted, bool, Succeeded);

UCLASS()
class RALLYHERESTART_API URHContextMenu : public URHWidget
{
	GENERATED_UCLASS_BODY()

public:
	
	virtual void InitializeWidget_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void SetCurrentFriend(class URH_RHFriendAndPlatformFriend* Friend);

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void AddContextMenuButton(class URHContextMenuButton* ContextButton);

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void RemoveContextMenuButton(class URHContextMenuButton* ContextButton);

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ClearAllContextMenuButton();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	class URHContextMenuButton* GetContextButtonByEnum(EPlayerContextOptions ContextOption);

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	bool OnOptionSelected(EPlayerContextOptions ContextOption);

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void SetOptionsVisibility();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	FVector2D SetMenuPosition(class URHWidget* WidgetToMove, FMargin WidgetPadding, EViewSide side, float menuWidth, float menuHeight);

protected:
	bool ExecuteSelectedOption(EPlayerContextOptions ContextOption);
	bool AttemptFriendAction(EFriendAction FriendAction);
	bool PerformFriendAction(EFriendAction FriendAction, FGuid FriendUuid);
	bool AttemptPartyAction(EPartyManagerAction PartyAction);
	bool AttemptQueueAction(EQueueDataFactoryAction QueueAction);
	bool AttemptInviteToParty();
	bool AttemptMute(bool bMute);
	bool AttemptReportPlayer();
	bool AttemptIgnorePlayer(bool bIgnore);

	FVector2D CalcMenuPosition(FVector2D WidgetAbsoluteMin, FVector2D WidgetAbsoluteMax, float menuWidth, float menuHeight);

	void SetOptionsActive();
	void SetContextButtonActive(EPlayerContextOptions ContextOption, bool IsActive);
	void SetContextButtonVisibility(class URHContextMenuButton* ContextButton);

	UFUNCTION()
	void HandleOnQueueStatusChange(ERH_MatchStatus QueueStatus);

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnContextOptionsUpdated OnContextOptionsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnReportPlayer OnReportPlayer;

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnContextOptionCompleted OnContextOptionCompleted;

protected:
	URH_PlayerInfoSubsystem* GetRHPlayerInfoSubsystem() const;
	URH_FriendSubsystem* GetRHFriendSubsystem() const;

	UFUNCTION(BlueprintPure, Category = "Context Menu")
	URHPartyManager* GetPartyManager() const;

	UFUNCTION(BlueprintPure, Category = "Context Menu")
	URHQueueDataFactory* GetQueueDataFactory() const;

	UPROPERTY(EditAnywhere, Category = "Context Menu")
	EPlayerContextMenuContext MenuContext;

	UPROPERTY(EditAnywhere, Category = "Context Menu")
	bool bAllowReportPlayer;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	class URH_RHFriendAndPlatformFriend* CurrentFriend;

	UPROPERTY(BlueprintReadWrite, Category = "Context Menu")
	TArray<class URHContextMenuButton*> ContextMenuButtons;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	EViewSide MenuViewSide;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	int32 CachedQueuedOrInMatch;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	bool bCachedReportedPlayer;

private:
	bool IsLobbyOptionVisible(EPlayerContextOptions ContextOption);
	void UpdateFriendChecks();
	void UpdatePlayerChecks();

	bool IsInParty();
	bool IsPartyFull();
	bool HasPlayerInvited();
	bool IsPartyLeader();
	bool CanInvitePlayer();
	bool IsPlayerPendingPartyInvite();
	bool IsPlayerInParty();
	bool IsPlayerOnline();
	bool IsLocalPlayer();
	bool IsCrossplayEnabled();
	bool IsFriend();
	bool IsRHFriend();
	bool IsRequestingFriend();
	bool IsPendingFriend();
	bool IsMuted();
	bool IsInVoiceChannel();
	bool HasReportedPlayer();
	bool IsIgnored();
	bool IsBlockedByRH();
	bool IsBlockedByPlatform();
	bool AddRHFriend(const FGuid& PlayerId) const;
	UFUNCTION()
	void RemoveRHFriend();
	bool RemoveFriendRequest(const FGuid& PlayerId) const;
	bool PromptForFriendRemoval();
};

/*
* Player Context Option Button
*/
UCLASS()
class RALLYHERESTART_API URHContextMenuButton : public URHWidget
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Context Menu Button")
	void SetContextOption(EPlayerContextOptions Context);

	UFUNCTION(BlueprintPure, Category = "Context Menu Button")
	FORCEINLINE EPlayerContextOptions GetContextOption() const { return ContextOption; }

	UFUNCTION(BlueprintNativeEvent, Category = "Context Menu Button")
	void HandleVisibility(bool isVisibility);

	UFUNCTION(BlueprintNativeEvent, Category = "Context Menu Button")
	void HandleActive(bool isActive);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Menu Button", Meta = (ExposeOnSpawn = true))
	EPlayerContextOptions ContextOption;
};