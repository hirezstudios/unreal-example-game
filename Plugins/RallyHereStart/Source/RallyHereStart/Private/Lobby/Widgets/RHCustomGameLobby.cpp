#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Lobby/Widgets/RHMassInviteModal.h"
#include "Lobby/Widgets/RHCustomLobbyPlayer.h"
#include "RH_GameInstanceSubsystem.h"
#include "Lobby/Widgets/RHCustomGameLobby.h"

URHCustomGameLobby::URHCustomGameLobby(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	NumTeamPlayers(5),
	NumSpectators(2),
	LobbyPlayerWidgetClass(URHCustomLobbyPlayer::StaticClass()),
	SpectatorTeamNum(INDEX_NONE)
	//$$ KAB - Removed MassInviteRouteName Initializer, Route names changed to Gameplay Tags
{
}

void URHCustomGameLobby::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		QueueDataFactory->OnCustomMatchJoined.AddDynamic(this, &URHCustomGameLobby::PopulateLobbyWithSessionData);
		QueueDataFactory->OnCustomMatchDataChanged.AddDynamic(this, &URHCustomGameLobby::PopulateLobbyWithSessionData);
		QueueDataFactory->OnCustomMatchMapChanged.AddDynamic(this, &URHCustomGameLobby::HandleMapChanged);
	}

	PopulateLobbyWithSessionData();
	HandleMapChanged();

	// Context actions
	AddContextAction(FName("Back"));
	BackContextAction.BindDynamic(this, &URHCustomGameLobby::HandleBackContextAction);
	SetContextAction(FName("Back"), BackContextAction);
}

void URHCustomGameLobby::UninitializeWidget_Implementation()
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		QueueDataFactory->OnCustomMatchJoined.RemoveAll(this);
	}

	Super::UninitializeWidget_Implementation();
}

void URHCustomGameLobby::HandleBackContextAction()
{
	NavigateBack();
}

bool URHCustomGameLobby::NavigateBack_Implementation()
{
	RemoveTopViewRoute();
	return Super::NavigateBack_Implementation();
}

/* This needs TeamNumToTeamPanelMap and SpectatorTeamNum set */
void URHCustomGameLobby::SetUpTeamPlayerWidgets()
{
	if (LobbyPlayerWidgetClass == nullptr)
	{
		return;
	}

	TArray<int32> PanelTeamNums;
	TeamNumToTeamPanelMap.GetKeys(PanelTeamNums);

	//$$ DLF BEGIN - Added controller navigation support to the Custom Lobby
	int32 TeamIndex = 0;
	PlayerWidgets.Empty();
	PlayerWidgets.AddDefaulted(PanelTeamNums.Num());
	//$$ DLF END - Added controller navigation support to the Custom Lobby
	
	for (const int32 PanelTeamNum : PanelTeamNums)
	{
		// For normal teams (non-spectator), need 1 more valid widget that only shows up if it represents a player 
		// for ease of moving people around
		const int32 NumPlayerWidgets = (PanelTeamNum == SpectatorTeamNum) ? NumSpectators : NumTeamPlayers + 1;
		const int32 NumAlwaysVisible = (PanelTeamNum == SpectatorTeamNum) ? NumSpectators : NumTeamPlayers;

		if (UPanelWidget* TeamPanelWidget = TeamNumToTeamPanelMap[PanelTeamNum])
		{
			TeamPanelWidget->ClearChildren();

			for (int i = 0; i < NumPlayerWidgets; i++)
			{
				if (URHCustomLobbyPlayer* PlayerWidget = CreateWidget<URHCustomLobbyPlayer>(this, LobbyPlayerWidgetClass))
				{
					PlayerWidget->bHiddenWhenEmpty = i >= NumAlwaysVisible;
					PlayerWidget->InitializeWidget();
					PlayerWidget->AssociatedTeam = PanelTeamNum;
					PlayerWidget->SetEmpty();
					TeamPanelWidget->AddChild(PlayerWidget);

					PlayerWidget->OnCustomPlayerSwapTeamDel.AddDynamic(this, &URHCustomGameLobby::HandlePlayerSwapTeam);
					PlayerWidget->OnPlayerSelectedDel.AddDynamic(this, &URHCustomGameLobby::HandlePlayerClicked);
					PlayerWidget->OnEmptySelectedDel.AddDynamic(this, &URHCustomGameLobby::OpenMassInviteView);

					PlayerWidgets[TeamIndex].Add(PlayerWidget); //$$ DLF - Added controller navigation support to the Custom Lobby
				}
			}
		}

		TeamIndex++; //$$ DLF - Added controller navigation support to the Custom Lobby
	}

	//$$ DLF BEGIN - Added controller navigation support to the Custom Lobby
	URHWidget* MapButton = GetMapButton();
	URHWidget* StartGameButton = GetStartGameButton();
	URHWidget* SpectatorButton = GetSpectatorButton();
	URHWidget* LeaveLobbyButton = GetLeaveLobbyButton();

	for (TeamIndex = 0; TeamIndex < PlayerWidgets.Num(); TeamIndex++)
	{
		for (int32 PlayerIndex = 0; PlayerIndex < PlayerWidgets[TeamIndex].Num(); PlayerIndex++)
		{
			URHWidget *Up = nullptr, *Down = nullptr, *Left = nullptr, *Right = nullptr;
			if (PlayerIndex > 0)
			{
				Up = PlayerWidgets[TeamIndex][PlayerIndex - 1];
			}
			else if (TeamIndex == 0)
			{
				Up = MapButton;
			}
			else
			{
				Up = StartGameButton;
			}

			if (PlayerIndex < PlayerWidgets[TeamIndex].Num() - 1)
			{
				Down = PlayerWidgets[TeamIndex][PlayerIndex + 1];
			}
			else if (TeamIndex == 2)
			{
				Down = SpectatorButton;
			}

			if (TeamIndex > 0)
			{
				Left = PlayerWidgets[TeamIndex - 1][PlayerIndex];
			}

			if (TeamIndex == 0 || (TeamIndex == 1 && PlayerIndex < 2))
			{
				Right = PlayerWidgets[TeamIndex + 1][PlayerIndex];
			}
			else if (TeamIndex == 1 && PlayerIndex < 4)
			{
				Right = SpectatorButton;
			}
			else if (TeamIndex == 1)
			{
				Right = LeaveLobbyButton;
			}
			
			RegisterWidgetToInputManager(PlayerWidgets[TeamIndex][PlayerIndex], 0, Up, Down, Left, Right);
		}
	}

	if (PlayerWidgets.Num() > 0 && PlayerWidgets[0].Num() > 0)
	{
		SetDefaultFocusForGroup(PlayerWidgets[0][0], 0);
	}
	//$$ DLF END - Added controller navigation support to the Custom Lobby
}

void URHCustomGameLobby::PopulateLobbyWithSessionData_Implementation()
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		TArray<FRH_CustomMatchMember> PlayersData = QueueDataFactory->GetCustomMatchMembers();

		TeamNumToTeamStructMap.Reset();
		for (FRH_CustomMatchMember Player : PlayersData)
		{
			TArray<FRH_CustomMatchMember>& PlayerList = TeamNumToTeamStructMap.FindOrAdd(Player.TeamId).TeamMembers;
			PlayerList.AddUnique(Player);
		}

		// This is just to have some sort of consistency between refreshes
		SortPlayerListsInMap();
	}

	TArray<int32> PanelTeamNums;
	TeamNumToTeamPanelMap.GetKeys(PanelTeamNums);

	for (int32 TeamNum : PanelTeamNums)
	{
		if (TeamNumToTeamPanelMap[TeamNum] != nullptr)
		{
			bool bHasCorrespondingData = TeamNumToTeamStructMap.Contains(TeamNum);

			for (int i = 0; i < TeamNumToTeamPanelMap[TeamNum]->GetAllChildren().Num(); i++)
			{
				if (URHCustomLobbyPlayer* PlayerWidget = Cast<URHCustomLobbyPlayer>(TeamNumToTeamPanelMap[TeamNum]->GetAllChildren()[i]))
				{
					if (!bHasCorrespondingData || !TeamNumToTeamStructMap[TeamNum].TeamMembers.IsValidIndex(i))
					{
						// The team that this panel represents (TeamNum) has no data 
						// OR this team has no player at this index (i), so setting all player widgets to empty state
						PlayerWidget->SetEmpty();
					}
					else
					{
						PlayerWidget->SetMatchMember(TeamNumToTeamStructMap[TeamNum].TeamMembers[i]);
					}
				}
			}
		}
	}
}

void URHCustomGameLobby::HandlePlayerSwapTeam(FGuid PlayerUUID, int32 CurrentTeam)
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		TArray<int32> TeamNums;
		TeamNumToTeamPanelMap.GetKeys(TeamNums);

		int32 TeamToSwitchTo = -1;
		for (int TeamNum : TeamNums)
		{
			// Not current team and not spectator team
			if (TeamNum != CurrentTeam && TeamNum != SpectatorTeamNum)
			{
				TeamToSwitchTo = TeamNum;
				break;
			}
		}

		QueueDataFactory->SetPlayerTeamCustomMatch(PlayerUUID, TeamToSwitchTo);
	}
}

void URHCustomGameLobby::ToggleLocalPlayerSpectate()
{
	if (MyHud != nullptr)
	{
		if (URH_LocalPlayerSubsystem* RHSS = MyHud->GetLocalPlayerSubsystem())
		{
			if (IsLocalPlayerSpectator())
			{
				HandlePlayerSwapTeam(RHSS->GetPlayerUuid(), SpectatorTeamNum);
			}
			else
			{
				if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
				{
					QueueDataFactory->SetPlayerTeamCustomMatch(RHSS->GetPlayerUuid(), SpectatorTeamNum);
				}
			}
		}
	}
}

bool URHCustomGameLobby::IsLocalPlayerSpectator() const
{
	URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory();
	if (MyHud == nullptr || QueueDataFactory == nullptr)
	{
		return false;
	}

	if (URH_LocalPlayerSubsystem* RHSS = MyHud->GetLocalPlayerSubsystem())
	{
		return QueueDataFactory->GetPlayerTeamId(RHSS->GetPlayerUuid()) == SpectatorTeamNum;
	}

	return false;
}

URHQueueDataFactory* URHCustomGameLobby::GetQueueDataFactory() const
{
	if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
	{
		return RHLobbyHUD->GetQueueDataFactory();
	}
	return nullptr;
}

URH_JoinedSession* URHCustomGameLobby::GetCustomMatchSession() const
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		return QueueDataFactory->GetCustomMatchSession();
	}
	return nullptr;
}

//$$ DLF BEGIN - Added controller navigation support to the Custom Lobby
URHWidget* URHCustomGameLobby::GetPlayerWidget(int32 TeamIndex, int32 PlayerIndex) const
{
	if (TeamIndex < PlayerWidgets.Num())
	{
		if (PlayerIndex < 0 || PlayerIndex >= PlayerWidgets[TeamIndex].Num())
		{
			return PlayerWidgets[TeamIndex][PlayerWidgets[TeamIndex].Num() - 1];
		}

		return PlayerWidgets[TeamIndex][PlayerIndex];
	}
	
	return nullptr;
}
//$$ DLF END - Added controller navigation support to the Custom Lobby

void URHCustomGameLobby::SortPlayerListsInMap()
{
	TArray<int32> TeamNums;
	TeamNumToTeamStructMap.GetKeys(TeamNums);

	for (int32 TeamNum : TeamNums)
	{
		TArray<FRH_CustomMatchMember>& PlayerList = TeamNumToTeamStructMap[TeamNum].TeamMembers;
		PlayerList.Sort([](const FRH_CustomMatchMember& p1, const FRH_CustomMatchMember& p2)
			{
				return p1.PlayerUUID < p2.PlayerUUID;
			});
	}
}

#pragma region MASS INVITE FUNCTIONS
void URHCustomGameLobby::OpenMassInviteView(int32 InTeamToInviteTo)
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		if (QueueDataFactory->IsLocalPlayerCustomLobbyLeader())
		{
			URHDataIndividualInviteSetup* InviteViewData = NewObject<URHDataIndividualInviteSetup>(MyHud.Get());
			InviteViewData->Title = NSLOCTEXT("RHCustomGames", "InviteToMatch", "InviteToMatch");

			URHDataIndividualInviteSetup::FRHInviteGetIsSelected IsSelectedDel;
			IsSelectedDel.BindDynamic(this, &URHCustomGameLobby::MassInvite_IsSelected);
			URHDataIndividualInviteSetup::FRHInviteSelect SelectDel;
			SelectDel.BindDynamic(this, &URHCustomGameLobby::MassInvite_Select);
			FRHInviteShouldShowPlayer ShouldShowPlayerDel;
			ShouldShowPlayerDel.BindDynamic(this, &URHCustomGameLobby::MassInvite_ShouldShowPlayer);
			FRHInviteCancel InviteCancelDel;
			InviteCancelDel.BindDynamic(this, &URHCustomGameLobby::MassInvite_Close);

			InviteViewData->SetCallbacks(IsSelectedDel, SelectDel, ShouldShowPlayerDel, InviteCancelDel);

			AddViewRoute(MassInviteRouteTag, false, false, InviteViewData); // $$ KAB - Route names changed to Gameplay Tags

			TeamToInviteTo = InTeamToInviteTo;
		}
	}
}

bool URHCustomGameLobby::MassInvite_IsSelected(URH_RHFriendAndPlatformFriend* PlayerInfo)
{
	URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory();
	if (PlayerInfo == nullptr || QueueDataFactory == nullptr)
	{
		return false;
	}
	return QueueDataFactory->IsCustomInvitePending(PlayerInfo->GetRHPlayerUuid());
}

bool URHCustomGameLobby::MassInvite_ShouldShowPlayer(URH_RHFriendAndPlatformFriend* PlayerInfo)
{
	URH_PlayerInfoSubsystem* PIS = nullptr;

	if (MyHud != nullptr)
	{
		if (URH_LocalPlayerSubsystem* RHSS = MyHud->GetLocalPlayerSubsystem())
		{
			PIS = RHSS->GetPlayerInfoSubsystem();
		}
	}
	
	if (PlayerInfo != nullptr)
	{
		if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
		{
			if (!QueueDataFactory->IsPlayerInCustomMatch(PlayerInfo->GetRHPlayerUuid()))
			{
				bool bPresenceOnline = false;
				if (PIS != nullptr)
				{
					if (const URH_PlayerInfo* FriendPlayerInfo = PIS->GetPlayerInfo(PlayerInfo->GetRHPlayerUuid()))
					{
						if (const URH_PlayerPresence* FriendPresence = FriendPlayerInfo->GetPresence())
						{
							bPresenceOnline = FriendPresence->Status == ERHAPI_OnlineStatus::Online;
						}
					}
				}

				if (bPresenceOnline || PlayerInfo->IsOnline())
				{
					return true;
				}
			}
		}
	}

	return false;
}

ERHInviteSelectResult URHCustomGameLobby::MassInvite_Select(URH_RHFriendAndPlatformFriend* PlayerInfo)
{
	URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory();
	if (PlayerInfo == nullptr || QueueDataFactory == nullptr || QueueDataFactory->IsCustomInvitePending(PlayerInfo->GetRHPlayerUuid()))
	{
		return ERHInviteSelectResult::NoChange;
	}

	QueueDataFactory->InviteToCustomMatch(PlayerInfo->GetRHPlayerUuid(), TeamToInviteTo);
	return ERHInviteSelectResult::Selected;
}

void URHCustomGameLobby::MassInvite_Close_Implementation()
{
	TeamToInviteTo = 0;
}
#pragma endregion