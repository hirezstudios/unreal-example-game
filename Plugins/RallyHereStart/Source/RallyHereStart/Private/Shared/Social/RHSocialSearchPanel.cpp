// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Shared/Social/RHSocialSearchPanel.h"
#include "Shared/Social/RHDataSocialCategory.h"
#include "Shared/Widgets/RHSocialOverlay.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "RH_GameInstanceSubsystem.h"
#include "RH_PlayerInfoSubsystem.h"


void URHSocialSearchPanel::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	SetSections({
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::SearchResults,	ERHSocialPanelDisplayOption::HideIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::Pending,			ERHSocialPanelDisplayOption::HideIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::RecentlyPlayed,	ERHSocialPanelDisplayOption::ShowIfEmpty, ERHCategoryOpenMode::OpenByDefault},
		FRHSocialPanelSectionDef{ERHSocialOverlaySection::SuggestedFriends,	ERHSocialPanelDisplayOption::ShowIfEmpty, ERHCategoryOpenMode::OpenByDefault}
	});

	OnDataChange();
}

void URHSocialSearchPanel::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();
}

void URHSocialSearchPanel::DoSearch(FText SearchTerm)
{
	if (SearchTerm.EqualTo(ActiveSearchTerm) || SearchTerm.EqualTo(FText::GetEmpty())) // prevent dupes/empty searches
	{
		return;
	}

	if (!MyHud.IsValid())
	{
		return;
	}

	URH_PlayerInfoSubsystem* RHPI = MyHud->GetPlayerInfoSubsystem();
	if (RHPI == nullptr)
	{
		UE_LOG(RallyHereStart, Error, TEXT("RHSocialSearchPanel::DoSearch Failed to get player info subsystem"));
		return;
	}

	// clear current results
	if (auto* Data = GetDataSource())
	{
		if (URHDataSocialCategory* Category = Data->GetCategory(ERHSocialOverlaySection::SearchResults))
		{
			Category->Empty(5);
			Category->SetMessageText(true, NSLOCTEXT("RHSocial", "SearchInProgress", "Searching"));
			Category->OnPlayersUpdated.Broadcast(Category, Category->GetPlayerList());
			OnDataChange({FRHSocialOverlaySectionInfo(ERHSocialOverlaySection::SearchResults)});
		}
	}

	ActiveSearchTerm = SearchTerm;

	RHPI->LookupPlayer(SearchTerm.ToString(), FRH_PlayerInfoLookupPlayerDelegate::CreateLambda([this, SearchTerm](bool bSuccess, const TArray<URH_PlayerInfo*>& PlayerInfos) -> void
		{
			// Only display results if the results are for the last executed search
			if (ActiveSearchTerm.EqualTo(SearchTerm))
			{
				if (auto* Data = GetDataSource())
				{
					if (bSuccess)
					{
						if (URHDataSocialCategory* Category = Data->GetCategory(ERHSocialOverlaySection::SearchResults))
						{
							if (URH_FriendSubsystem* FSS = GetFriendSubsystem())
							{
								Category->SetMessageText(false, FText::GetEmpty());
								Category->SetOpenOnUpdate(true);
								int Count = 0;
								for (URH_PlayerInfo* PlayerInfo : PlayerInfos)
								{
									UE_LOG(RallyHereStart, Log, TEXT("RHSocialSearchPanel::FRH_PlayerInfoLookupPlayerDelegate Player Id Found: %s"), *PlayerInfo->GetRHPlayerUuid().ToString());

									if (auto* Container = NewObject<URHDataSocialPlayer>(MyHud.Get()))
									{
										if (Count++ < VisiblePlayerCount)
										{
											PlayerInfo->GetPresence()->RequestUpdate(true);
										}
										
										Container->SetFriend(FSS->GetOrCreateFriend(PlayerInfo));
										Category->Add(Container);
										Category->OnPlayersUpdated.Broadcast(Category, Category->GetPlayerList());
									}
								}
							}
						}
						OnDataChange({ FRHSocialOverlaySectionInfo(ERHSocialOverlaySection::SearchResults) });
					}
					else
					{
						if (auto* Category = Data->GetCategory(ERHSocialOverlaySection::SearchResults))
						{
							Category->SetMessageText(false, NSLOCTEXT("RHSocial", "SearchNoResults", "No Results"));
							Category->OnPlayersUpdated.Broadcast(Category, Category->GetPlayerList());
							OnSearchComplete(SearchTerm, NSLOCTEXT("RHSocial", "SearchNoResults", "No Results"), Category->GetPlayerList());
						}
					}
				}
			}
		}));
}

void URHSocialSearchPanel::OnOverlayClosed()
{
	ActiveSearchTerm = FText::GetEmpty();
}

URH_FriendSubsystem* URHSocialSearchPanel::GetFriendSubsystem() const
{
	if (MyHud.IsValid())
	{
		if (const auto RHSS = MyHud->GetLocalPlayerSubsystem())
		{
			return RHSS->GetFriendSubsystem();
		}
	}

	return nullptr;
}