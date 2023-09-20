// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
//#include "RHLobbyHUD.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RHPartyManagerWidgetBase.h"

void URHPartyManagerWidgetBase::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	if (URHPartyManager* pPartyManager = GetPartyManager())
	{
		pPartyManager->OnPartyDataUpdated.AddDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);
		pPartyManager->OnPartyInvitationAccepted.AddDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);
		pPartyManager->OnPartyInvitationRejected.AddDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);
		pPartyManager->OnPartyInvitationExpired.AddDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);

		pPartyManager->OnPartyMemberPromoted.AddDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateById);
		pPartyManager->OnPartyMemberDataUpdated.AddDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberDataUpdated);
		pPartyManager->OnPendingPartyMemberDataAdded.AddDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberDataUpdated);
		pPartyManager->OnPartyMemberRemoved.AddDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateById);
		pPartyManager->OnPartyMemberLeft.AddDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberDataUpdated);
		pPartyManager->OnPartyInvitationSent.AddDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateByInfo);
		pPartyManager->OnPartyInvitationReceived.AddDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateByInfo);
	}
	RefreshFromPartyData();
}

void URHPartyManagerWidgetBase::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();

	if (URHPartyManager* pPartyManager = GetPartyManager())
	{
		pPartyManager->OnPartyDataUpdated.RemoveDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);
		pPartyManager->OnPartyInvitationAccepted.RemoveDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);
		pPartyManager->OnPartyInvitationRejected.RemoveDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);
		pPartyManager->OnPartyInvitationExpired.RemoveDynamic(this, &URHPartyManagerWidgetBase::RefreshFromPartyData);

		pPartyManager->OnPartyMemberPromoted.RemoveDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateById);
		pPartyManager->OnPartyMemberDataUpdated.RemoveDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberDataUpdated);
		pPartyManager->OnPendingPartyMemberDataAdded.RemoveDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberDataUpdated);
		pPartyManager->OnPartyMemberRemoved.RemoveDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateById);
		pPartyManager->OnPartyMemberLeft.RemoveDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberDataUpdated);
		pPartyManager->OnPartyInvitationSent.RemoveDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateByInfo);
		pPartyManager->OnPartyInvitationReceived.RemoveDynamic(this, &URHPartyManagerWidgetBase::HandlePartyMemberUpdateByInfo);
	}
}

URHPartyManager* URHPartyManagerWidgetBase::GetPartyManager()
{
	if (ARHHUDCommon* RHHUDCommon = Cast<ARHHUDCommon>(MyHud))
	{
		return RHHUDCommon->GetPartyManager();
	}

	return nullptr;
}

void URHPartyManagerWidgetBase::RefreshFromPartyData()
{
	if (URHPartyManager* pPartyManager = GetPartyManager())
	{
		TArray<FRH_PartyMemberData> PartyMembers = pPartyManager->GetPartyMembers();

		CachedDisplayedPartyMembers = PartyMembers;
		ApplyPartyData(PartyMembers);
	}
}

void URHPartyManagerWidgetBase::ApplyEmptyPartyData()
{
	CachedDisplayedPartyMembers = TArray<FRH_PartyMemberData>();
	ApplyPartyData(CachedDisplayedPartyMembers);
}

void URHPartyManagerWidgetBase::HandlePartyMemberDataUpdated(FRH_PartyMemberData MemberData)
{
	RefreshFromPartyData();
}

void URHPartyManagerWidgetBase::HandlePartyMemberUpdateById(const FGuid& PlayerId)
{
	RefreshFromPartyData();
}

void URHPartyManagerWidgetBase::HandlePartyMemberUpdateByName(FText PlayerName)
{
	RefreshFromPartyData();
}

void URHPartyManagerWidgetBase::HandlePartyMemberUpdateByInfo(URH_PlayerInfo* PlayerInfo)
{
	RefreshFromPartyData();
}

URH_PlayerInfo* URHPartyManagerWidgetBase::GetSuggestedInvite()
{
	// To do: routine for picking a player to recommend
	return nullptr;
}