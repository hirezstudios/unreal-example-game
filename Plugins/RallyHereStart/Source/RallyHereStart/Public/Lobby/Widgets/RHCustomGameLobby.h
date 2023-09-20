#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "RHCustomGameLobby.generated.h"

/*
* This is in order to put an array into a map
*/
USTRUCT(BlueprintType)
struct FCustomLobbyTeam
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TArray<FRH_CustomMatchMember> TeamMembers;
};

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHCustomGameLobby : public URHWidget
{
	GENERATED_BODY()

	URHCustomGameLobby(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

protected:
	virtual bool NavigateBack_Implementation() override;

	FOnContextAction BackContextAction;

	UFUNCTION()
	void HandleBackContextAction();
	
protected:
#pragma region MEMBER VARIABLES
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Lobby")
	int32 NumTeamPlayers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Lobby")
	int32 NumSpectators;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Lobby")
    TSubclassOf<class URHCustomLobbyPlayer> LobbyPlayerWidgetClass;

	/* Map of team nums to panel widget, to be set in BP */
	UPROPERTY(BlueprintReadWrite, Category = "Custom Lobby")
	TMap<int32, UPanelWidget*> TeamNumToTeamPanelMap;

	/* Which team num the spectator team corresponds to (in TeamNumToTeamPanelMap), to be set in BP */
	UPROPERTY(BlueprintReadWrite, Category = "Custom Lobby")
	int32 SpectatorTeamNum;

	/* Map of team nums to players, transient */
	UPROPERTY(BlueprintReadOnly, Category = "Custom Lobby")
	TMap<int32, FCustomLobbyTeam> TeamNumToTeamStructMap;
#pragma endregion

#pragma region HELPER GETTERS
	UFUNCTION(BlueprintPure, Category = "Custom Lobby|Helper")
	bool IsLocalPlayerSpectator() const;
	
	UFUNCTION(BlueprintPure, Category = "Custom Lobby|Helper")
	class URHQueueDataFactory* GetQueueDataFactory() const;

	UFUNCTION(BlueprintPure, Category = "Custom Lobby|Helper")
	URH_JoinedSession* GetCustomMatchSession() const;
#pragma endregion

	/* To be called after setting TeamNumToTeamPanelMap and SpectatorTeamNum */
	UFUNCTION(BlueprintCallable, Category = "Custom Lobby")
	void SetUpTeamPlayerWidgets();

	UFUNCTION(BlueprintNativeEvent, Category = "Custom Lobby")
	void PopulateLobbyWithSessionData();

	UFUNCTION(BlueprintCallable, Category = "Custom Lobby")
	void ToggleLocalPlayerSpectate();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Custom Lobby")
	void HandleMapChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Custom Lobby|Player Widget")
	void HandlePlayerClicked(FRH_CustomMatchMember MatchMember, class URHCustomLobbyPlayer* PlayerWidget);

private:
	/* Swap between teams of players only */
	UFUNCTION()
	void HandlePlayerSwapTeam(FGuid PlayerUUID, int32 CurrentTeam);

	void SortPlayerListsInMap();

#pragma region MASS INVITE
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Lobby|Mass Invite")
	FName MassInviteRouteName;

private:
	/* Transient */
	int32 TeamToInviteTo;

	UFUNCTION()
	void OpenMassInviteView(int32 InTeamToInviteTo);

	UFUNCTION()
	bool MassInvite_IsSelected(URH_RHFriendAndPlatformFriend* PlayerInfo);

	UFUNCTION()
	bool MassInvite_ShouldShowPlayer(URH_RHFriendAndPlatformFriend* PlayerInfo);

	UFUNCTION()
	enum ERHInviteSelectResult MassInvite_Select(URH_RHFriendAndPlatformFriend* PlayerInfo);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Custom Lobby|Mass Invite")
	void MassInvite_Close();
#pragma endregion
};
