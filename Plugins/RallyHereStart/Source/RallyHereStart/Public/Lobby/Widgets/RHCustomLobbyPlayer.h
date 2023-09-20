#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHCustomLobbyPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomPlayerSwapTeam, FGuid, PlayerUUID, int32, CurrentTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerSelected, FRH_CustomMatchMember, MatchMember, URHCustomLobbyPlayer*, PlayerWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmptySelected, int32, AssociatedTeam);

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHCustomLobbyPlayer : public URHWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Custom Lobby Player", Meta = (ExposeOnSpawn = true))
	bool bHiddenWhenEmpty;

	/* Which team panel is this displayed in. For helping with invites. */
	UPROPERTY(BlueprintReadOnly, Category = "Custom Lobby Player")
	int32 AssociatedTeam;

	FOnCustomPlayerSwapTeam OnCustomPlayerSwapTeamDel;
	FOnPlayerSelected OnPlayerSelectedDel;
	FOnEmptySelected OnEmptySelectedDel;

	UFUNCTION(BlueprintNativeEvent, Category = "Custom Lobby Player")
	void SetMatchMember(FRH_CustomMatchMember InMatchMember);
	virtual void SetMatchMember_Implementation(FRH_CustomMatchMember InMatchMember);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Custom Lobby Player")
	void SetEmpty();
	virtual void SetEmpty_Implementation();

protected:
	UPROPERTY(BlueprintReadOnly)
	FRH_CustomMatchMember MatchMemberInfo;

	/* Swap between teams as players (not to spectator team) */
	UFUNCTION(BlueprintCallable, Category = "Custom Lobby Player|Actions")
	void SwapTeam() 
	{ 
		OnCustomPlayerSwapTeamDel.Broadcast(MatchMemberInfo.PlayerUUID, MatchMemberInfo.TeamId); 
	};

	UFUNCTION(BlueprintCallable, Category = "Custom Lobby Player|Actions")
	void PlayerClicked()
	{
		OnPlayerSelectedDel.Broadcast(MatchMemberInfo, this);
	}

	/* Call when player widget is clicked in empty state */
	UFUNCTION(BlueprintCallable, Category = "Custom Lobby Player|Actions")
	void EmptyClicked()
	{
		OnEmptySelectedDel.Broadcast(AssociatedTeam);
	};
	
	UFUNCTION(BlueprintPure, Category = "Custom Lobby Player|Helper")
	bool GetCanLocalPlayerControl() const;

	UFUNCTION(BlueprintPure, Category = "Custom Lobby Player|Helper")
	bool GetCanLocalPlayerKick() const;

	UFUNCTION(BlueprintPure, Category = "Custom Lobby Player|Helper")
	class URHQueueDataFactory* GetQueueDataFactory() const;
};
