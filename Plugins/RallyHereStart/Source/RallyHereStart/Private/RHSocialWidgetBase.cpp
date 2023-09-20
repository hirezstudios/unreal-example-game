// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "RH_GameInstanceSubsystem.h"
#include "RHSocialWidgetBase.h"

void URHSocialWidgetBase::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	if (auto rhFriendSS = GetRH_LocalPlayerFriendSubsystem())
	{
		rhFriendSS->FriendListUpdatedDelegate.AddUObject(this, &URHSocialWidgetBase::RHUpdateFriends);
		rhFriendSS->FriendUpdatedDelegate.AddUObject(this, &URHSocialWidgetBase::RHUpdateFriend);
	}
	HandleFriendDataUpdated();

	GetPartyManager()->OnPartyDataUpdated.AddDynamic(this, &URHSocialWidgetBase::HandlePartyDataUpdated);
	GetPartyManager()->OnPartyMemberDataUpdated.AddDynamic(this, &URHSocialWidgetBase::HandleSpecificPartyDataUpdated);
	GetPartyManager()->OnPendingPartyMemberDataAdded.AddDynamic(this, &URHSocialWidgetBase::HandleSpecificPartyDataAdded);
	GetPartyManager()->OnPartyMemberRemoved.AddDynamic(this, &URHSocialWidgetBase::HandleSpecificPartyIdDataUpdated);
	GetPartyManager()->OnPartyMemberLeft.AddDynamic(this, &URHSocialWidgetBase::HandleSpecificPartyDataAdded);
	GetPartyManager()->OnPartyMemberStatusChanged.AddDynamic(this, &URHSocialWidgetBase::HandleSpecificPartyIdDataUpdated);
	HandlePartyDataUpdated();
}

void URHSocialWidgetBase::UninitializeWidget_Implementation()
{
	if (auto rhFriendSS = GetRH_LocalPlayerFriendSubsystem())
	{
		rhFriendSS->FriendListUpdatedDelegate.RemoveAll(this);
	}
}

void URHSocialWidgetBase::RHUpdateFriends(const TArray<URH_RHFriendAndPlatformFriend*>& UpdatedFriends)
{
	HandleFriendDataUpdated();
}

void URHSocialWidgetBase::RHUpdateFriend(URH_RHFriendAndPlatformFriend* UpdatedFriend)
{
	HandleFriendDataUpdated();
}

int32 URHSocialWidgetBase::GetOnlineFriendCount() const
{
	int32 OnlineCount = 0;

	if (const auto FSS = GetRH_LocalPlayerFriendSubsystem())
	{
		if (const auto PSS = GetRH_PlayerInfoSubsystem())
		{
			for (const auto Friend : FSS->GetFriends())
			{
				// Skip yourself
				if (MyHud != nullptr)
				{
					if (MyHud->GetLocalPlayerUuid() == Friend->GetRHPlayerUuid())
					{
						continue;
					}
				}

				const URH_PlayerPresence* Presence = nullptr;
				if (URH_PlayerInfo* PlayerInfo = PSS->GetPlayerInfo(Friend->GetRHPlayerUuid()))
				{
					Presence = PlayerInfo->GetPresence();
				}

				// Is online in platform or online RH friend
				if (Friend->IsOnline() || (Presence && (Presence->Status == ERHAPI_OnlineStatus::Online || Presence->Status == ERHAPI_OnlineStatus::Away)))
				{
					OnlineCount++;
				}
			}
		}
	}

	return OnlineCount;
}

int32 URHSocialWidgetBase::GetIncomingInvitesCount() const
{
	int32 InvitesCount = 0;

	if (const auto FSS = GetRH_LocalPlayerFriendSubsystem())
	{
		for (const auto Friend : FSS->GetFriends())
		{
			if (Friend->OtherIsAwaitingFriendshipResponse())
			{
				InvitesCount++;
			}
		}
	}

	return InvitesCount;
}

void URHSocialWidgetBase::HandleSpecificPartyDataAdded(FRH_PartyMemberData PartyMember)
{
	HandlePartyDataUpdated();
}

void URHSocialWidgetBase::HandleSpecificPartyDataUpdated(FRH_PartyMemberData PartyMember)
{
	HandlePartyDataUpdated();
}

void URHSocialWidgetBase::HandleSpecificPartyIdDataUpdated(const FGuid& PlayerId)
{
	HandlePartyDataUpdated();
}

URHPartyManager* URHSocialWidgetBase::GetPartyManager()
{
	if (MyHud.IsValid())
	{
		return MyHud->GetPartyManager();
	}

	return nullptr;
}

URH_FriendSubsystem* URHSocialWidgetBase::GetRH_LocalPlayerFriendSubsystem() const
{
	if (!MyHud.IsValid())
	{
		return nullptr;
	}

	APlayerController* PC = MyHud->GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] Failed to get owning player controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] Failed to get local player for controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	URH_LocalPlayerSubsystem* RHSS = LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>();
	if (!RHSS)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] RallyHere failed to get friend subsystem for controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	return RHSS->GetFriendSubsystem();
}

URH_PlayerInfoSubsystem* URHSocialWidgetBase::GetRH_PlayerInfoSubsystem() const
{
	if (!MyHud.IsValid())
	{
		return nullptr;
	}

	APlayerController* PC = MyHud->GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] Failed to get owning player controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] Failed to get local player for controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	URH_LocalPlayerSubsystem* RHSS = LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>();
	if (!RHSS)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] RallyHere failed to get friend subsystem for controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	return RHSS->GetPlayerInfoSubsystem();
}