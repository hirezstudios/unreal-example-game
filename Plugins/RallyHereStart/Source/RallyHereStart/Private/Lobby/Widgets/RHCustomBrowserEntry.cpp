// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Lobby/HUD/RHLobbyHUD.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "Lobby/Widgets/RHCustomBrowserEntry.h"

URHCustomBrowserEntry::URHCustomBrowserEntry(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URHCustomBrowserEntry::SetCustomGameInfo_Implementation(class URH_SessionView* InSession)
{
	CorrespondingSession = InSession;
}

bool URHCustomBrowserEntry::JoinGame()
{
	if (auto QueueDataFactory = GetQueueDataFactory())
	{
		return QueueDataFactory->JoinCustomMatchSession(CorrespondingSession);
	}
	return false;
}

FString URHCustomBrowserEntry::GetGameDisplayName() const
{
	FString ResultName = FString();
	if (CorrespondingSession)
	{
		if (auto QueueDataFactory = GetQueueDataFactory())
		{
			FString LeaderName;
			if (QueueDataFactory->GetBrowserSessionsLeaderName(CorrespondingSession, LeaderName))
			{
				ResultName += LeaderName + TEXT("\'s Game ");
			}
		}
		ResultName += TEXT("(") + CorrespondingSession->GetSessionId() + TEXT(")");
	}
	return ResultName;
}

int32 URHCustomBrowserEntry::GetPlayerCount() const
{
	if (CorrespondingSession)
	{
		return CorrespondingSession->GetSessionPlayerCount();
	}
	return INDEX_NONE;
}

int32 URHCustomBrowserEntry::GetMaxPlayerCount() const
{
	if (CorrespondingSession)
	{
		FRHAPI_SessionTemplate SessionTemplate = CorrespondingSession->GetTemplate();
		int32* NumTeams = SessionTemplate.GetNumTeamsOrNull();
		int32* PlayersPerTeam = SessionTemplate.GetPlayersPerTeamOrNull();
		if (NumTeams != nullptr && PlayersPerTeam != nullptr)
		{
			return (*NumTeams) * (*PlayersPerTeam);
		}
	}
	return INDEX_NONE;
}

URHQueueDataFactory* URHCustomBrowserEntry::GetQueueDataFactory() const
{
	if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
	{
		return RHLobbyHUD->GetQueueDataFactory();
	}
	return nullptr;
}