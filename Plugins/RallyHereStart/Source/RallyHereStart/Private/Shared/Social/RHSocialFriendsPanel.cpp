// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Social/RHSocialFriendsPanel.h"
#include "Shared/Social/RHDataSocialCategory.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/HUD/RHLobbyHUD.h"

URHSocialFriendsPanel::URHSocialFriendsPanel() : URHSocialPanelBase()
{
}

void URHSocialFriendsPanel::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	SetSections({
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::SessionMembers		, ERHSocialPanelDisplayOption::HideIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::SessionInvitations	, ERHSocialPanelDisplayOption::HideIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::FriendInvites			, ERHSocialPanelDisplayOption::HideIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::OnlineRHFriends		, ERHSocialPanelDisplayOption::ShowIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::OnlinePlatformFriends	, ERHSocialPanelDisplayOption::ShowIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::OfflineRHFriends		, ERHSocialPanelDisplayOption::ShowIfEmpty, ERHCategoryOpenMode::ClosedByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::Blocked				, ERHSocialPanelDisplayOption::HideIfEmpty, ERHCategoryOpenMode::ClosedByDefault}
	});

	OnDataChange();
}

void URHSocialFriendsPanel::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();
}