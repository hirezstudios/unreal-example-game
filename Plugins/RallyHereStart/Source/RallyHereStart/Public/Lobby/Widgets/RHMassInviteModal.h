#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RH_FriendSubsystem.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RHMassInviteModal.generated.h"

class URH_PlayerInfo;

UENUM(BlueprintType)
enum class ERHInviteSelectResult : uint8
{
	NoChange,
	Selected,
	Deselected
};

UENUM(BlueprintType)
enum class ERHInviteCloseAction : uint8
{
	None,
	Submit
};

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FRHInviteShouldShowPlayer, URH_RHFriendAndPlatformFriend*, PlayerAndPlatformInfo);
DECLARE_DYNAMIC_DELEGATE(FRHInviteCancel);

/*
* Base route data item for mass invite modal, not to be constructed directly.
*	Prefer: RHDataIndividualInviteSetup or RHDataBatchInviteSetup
*/
UCLASS(BlueprintType, Abstract)
class RALLYHERESTART_API URHDataMassInviteBase : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// Title to show on the invite modal
	UPROPERTY(BlueprintReadWrite,EditAnywhere,meta=(ExposeOnSpawn="true"), Category = "Mass Player Invite")
	FText Title;

	// Text to show on the button. Defaults to "Done" if empty
	UPROPERTY(BlueprintReadWrite,EditAnywhere,meta=(ExposeOnSpawn="true"), Category = "Mass Player Invite")
	FText ButtonLabel;

	// Callback to call for pruning friend data
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Mass Player Invite")
	FRHInviteShouldShowPlayer OnShouldShow;
	
	// Callback to call when the screen closes - doesn't do anything for already invited players
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Mass Player Invite")
	FRHInviteCancel OnClose;
};

/*
* Data item for RHMassInviteModal where clicking on a player
* 	immediately does an action
*/
UCLASS(BlueprintType)
class RALLYHERESTART_API URHDataIndividualInviteSetup : public URHDataMassInviteBase
{
	GENERATED_UCLASS_BODY()

public:

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FRHInviteGetIsSelected, URH_RHFriendAndPlatformFriend*, PlayerInfo);

	//
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(ERHInviteSelectResult, FRHInviteSelect, URH_RHFriendAndPlatformFriend*, PlayerInfo);


	// Callback to call to see if a data item is already selected
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Mass Player Invite")
	FRHInviteGetIsSelected OnGetIsSelected;

	// Callback to call for the main per-item select action
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Mass Player Invite")
	FRHInviteSelect OnSelect;

	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	URHDataIndividualInviteSetup* SetCallbacks(FRHInviteGetIsSelected GetIsSelected, FRHInviteSelect Select, FRHInviteShouldShowPlayer ShouldShowPlayer, FRHInviteCancel Close);
};

/*
* Data item for RHMassInviteModal where clicking on a player groups them up
*	for one big mass action in the end
*/
UCLASS(BlueprintType)
class RALLYHERESTART_API URHDataBatchInviteSetup : public URHDataMassInviteBase
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FRHBatchSelect, const TArray<URH_RHFriendAndPlatformFriend*>, Friends);

	// Callback to call when the player hits the main action button after selecting multiple players
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Mass Player Invite")
	FRHBatchSelect OnSelect;

	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	URHDataBatchInviteSetup* SetCallbacks(FRHBatchSelect Select, FRHInviteShouldShowPlayer ShouldShowPlayer, FRHInviteCancel Cancel);
};

UCLASS()
class RALLYHERESTART_API URHMassInviteModal : public URHWidget
{
	GENERATED_UCLASS_BODY()
public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FRHInviteReceivePlayers, const TArray<URH_RHFriendAndPlatformFriend*>&, PlayerAndPlatformInfos);


protected:
	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	void RequestFriends(FRHInviteReceivePlayers OnReceivePlayers);

	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	bool UpdateRouteData();

	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	bool GetShouldSelect(URH_RHFriendAndPlatformFriend* Friend);

	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	ERHInviteSelectResult SelectPlayer(URH_RHFriendAndPlatformFriend* Friend);

	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	void CloseScreen(ERHInviteCloseAction CloseAction);

	UFUNCTION(BlueprintCallable, Category = "Mass Player Invite")
	void DoSearch(FText SearchTerm);

	UFUNCTION(BlueprintImplementableEvent, Category = "Mass Player Invite")
	void OnSearchResultUpdated(const TArray<URH_RHFriendAndPlatformFriend*>& ResultPlayers);

	UPROPERTY(BlueprintReadOnly, Category = "Mass Player Invite")
	TArray<URH_RHFriendAndPlatformFriend*> SelectedFriends; // only used for Batch invites

	UPROPERTY(BlueprintReadOnly, Category = "Mass Player Invite")
	TWeakObjectPtr<URHDataMassInviteBase> RouteData;

private:
	URH_LocalPlayerSubsystem* GetRHLocalPlayerSubsystem() const;
	URH_FriendSubsystem* GetRHFriendSubsystem() const;
	
	void CheckAndAddPlayerToSearchResult(URH_PlayerInfo* PlayerInfo);

	UPROPERTY()
	TArray<URH_RHFriendAndPlatformFriend*> FriendResults;

	UPROPERTY()
	TArray<URH_RHFriendAndPlatformFriend*> SearchResult;
	
	FText ActiveSearchTerm;
};