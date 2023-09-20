#include "Lobby/HUD/RHLobbyHUD.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "Lobby/Widgets/RHCustomLobbyPlayer.h"

void URHCustomLobbyPlayer::SetMatchMember_Implementation(FRH_CustomMatchMember InMatchMember)
{
	MatchMemberInfo = InMatchMember;

	// do widget stuff
	SetVisibility(ESlateVisibility::Visible);
}

void URHCustomLobbyPlayer::SetEmpty_Implementation()
{
	SetMatchMember(FRH_CustomMatchMember());
	if (bHiddenWhenEmpty)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool URHCustomLobbyPlayer::GetCanLocalPlayerControl() const
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		return QueueDataFactory->CanLocalPlayerControlCustomLobbyPlayer(MatchMemberInfo.PlayerUUID);
	}
	return false;
}

bool URHCustomLobbyPlayer::GetCanLocalPlayerKick() const
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		return QueueDataFactory->CanLocalPlayerKickCustomLobbyPlayer(MatchMemberInfo.PlayerUUID);
	}
	return false;
}

URHQueueDataFactory* URHCustomLobbyPlayer::GetQueueDataFactory() const
{
	if (ARHLobbyHUD* LobbyHUD = Cast<ARHLobbyHUD>(MyHud))
	{
		return LobbyHUD->GetQueueDataFactory();
	}
	return nullptr;
}