// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Managers/RHStoreItemHelper.h"
#include "RH_CatalogSubsystem.h"
#include "RH_GameInstanceSubsystem.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/Widgets/RHStoreWidget.h"

void URHStoreWidget::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();

    if (URHStoreItemHelper* StoreItemHelper = GetStoreItemHelper())
    {
        StoreItemHelper->OnReceivePricePoints.AddDynamic(this, &URHStoreWidget::OnPricePointsReveived);
        StoreItemHelper->OnPortalOffersReceived.AddDynamic(this, &URHStoreWidget::OnPortalOffersReceived);
    }
}

void URHStoreWidget::UninitializeWidget_Implementation()
{
    Super::UninitializeWidget_Implementation();

    if (URHStoreItemHelper* StoreItemHelper = GetStoreItemHelper())
    {
        StoreItemHelper->ExitInGameStoreUI();
    }
}

TArray<URHStoreSection*> URHStoreWidget::GetStoreLayout(int32& ErrorCode)
{
	TArray<URHStoreSection*> RHStoreSections;
	URHStoreSection* NewSection;
	TArray<URHStoreItem*> Items;

	if (URHStoreItemHelper* StoreItemHelper = GetStoreItemHelper())
	{
		// This is just one example of ways we can parse out show panels, each product will need to determine how they want their store displayed and 
		// create their own store front/parsing of the store loot tables.

		static const int32 STORE_VENDOR_BUNDLES = 29;

		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (auto pGISubsystem = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
			{
				if (URH_CatalogSubsystem* CatalogSubsystem = pGISubsystem->GetCatalogSubsystem())
				{
					FRHAPI_Vendor Vendor;
					if (CatalogSubsystem->GetVendorById(StoreItemHelper->GetStoreVendorId(), Vendor))
					{
						int32 i = 0; // Exampling of different panel types
						if (const auto& LootItems = Vendor.GetLootOrNull())
						{
							for (const auto& pItemPair : (*LootItems))
							{
								const FRHAPI_Loot& LootItem = pItemPair.Value;

								if (const auto& SubVendorId = LootItem.GetSubVendorIdOrNull())
								{
									if (*SubVendorId != STORE_VENDOR_BUNDLES)
									{
										Items = StoreItemHelper->GetStoreItemsForVendor(*SubVendorId, false, true);
										if (Items.Num() > 0)
										{
											NewSection = CreateStoreSection(Items, EStoreSectionTypes::AccountCosmetics, i % 2 == 0 ? EStoreItemWidgetType::ESmallPanel : EStoreItemWidgetType::ETallPanel, i % 2 == 1);
											++i;

											if (NewSection != nullptr && NewSection->SectionItems.Num())
											{
												RHStoreSections.Push(NewSection);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		Items = StoreItemHelper->GetStoreItemsForVendor(STORE_VENDOR_BUNDLES, false, false);
		if (Items.Num() > 0)
		{
			NewSection = CreateStoreSection(Items, EStoreSectionTypes::Bundles, EStoreItemWidgetType::ETallPanel);

			if (NewSection != nullptr && NewSection->SectionItems.Num())
			{
				RHStoreSections.Push(NewSection);
			}
		}

		Items = StoreItemHelper->GetStoreItemsForVendor(StoreItemHelper->GetPortalOffersVendorId(), false, false);

		if (Items.Num() > 0)
		{
			NewSection = CreateStoreSection(Items, EStoreSectionTypes::PortalOffers, EStoreItemWidgetType::ETallPanel);

			if (NewSection != nullptr && NewSection->SectionItems.Num())
			{
				RHStoreSections.Push(NewSection);
			}
		}
	}

	// Kick off async loads of any assets we are using that are not in memory currently
	TArray<FSoftObjectPath> AssetsToLoad;

	for (URHStoreSection* StoreSection : RHStoreSections)
	{
		for (URHStoreSectionItem* SectionItem : StoreSection->SectionItems)
		{
			for (URHStorePanelItem* StorePanelItem : SectionItem->StorePanelItems)
			{
				if (StorePanelItem != nullptr && StorePanelItem->StoreItem != nullptr)
				{
					if (!StorePanelItem->StoreItem->GetInventoryItem().IsValid())
					{
						AssetsToLoad.Add(StorePanelItem->StoreItem->GetInventoryItem().ToSoftObjectPath());
					}
				}
			}
		}
	}

	if (AssetsToLoad.Num() > 0)
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetsToLoad);
	}

	return RHStoreSections;
}

URHStoreSection* URHStoreWidget::CreateStoreSection(TArray<URHStoreItem*>& StoreItems, EStoreSectionTypes SectionType, EStoreItemWidgetType PanelSize, bool SinglePanel /* = false */)
{
	URHStoreSection* CurrentSection = nullptr;
	URHStoreSectionItem* CurrentSectionItem = nullptr;
	URHStoreSectionItem* PreviousSectionItem = nullptr;

	CurrentSection = NewObject<URHStoreSection>();
	CurrentSection->SectionType = SectionType;
	CurrentSectionItem = nullptr;
	
	for (URHStoreItem* Item : StoreItems)
	{
		// Skip invalid items
		if (!Item)
		{
			continue;
		}

		if (CurrentSectionItem && CurrentSectionItem->StorePanelItems.Num())
		{
			PreviousSectionItem = CurrentSectionItem;
		}

		if (CurrentSectionItem == nullptr || !SinglePanel)
		{
			CurrentSectionItem = NewObject<URHStoreSectionItem>();
			CurrentSectionItem->WidgetType = PanelSize;

			// Calculate Row/Column/Display Type based on logic
			if (CurrentSectionItem->WidgetType == EStoreItemWidgetType::ESmallPanel)
			{
				// If the previous element is a small panel, and a top panel, we go under it
				if (PreviousSectionItem && PreviousSectionItem->WidgetType == EStoreItemWidgetType::ESmallPanel && PreviousSectionItem->Row == 0)
				{
					CurrentSectionItem->Column = PreviousSectionItem->Column;
					CurrentSectionItem->Row = 1;
				}
				else
				{
					CurrentSectionItem->Column = PreviousSectionItem ? PreviousSectionItem->Column + 1 : 0;
					CurrentSectionItem->Row = 0;
				}
			}
			else
			{
				CurrentSectionItem->Column = PreviousSectionItem ? PreviousSectionItem->Column + 1 : 0;
				CurrentSectionItem->Row = 0;
			}
		}

		// Only take items that are priced to display
		if (Item->HasPortalOffer() || Item->GetPrices().Num() > 0)
		{
			URHStorePanelItem* NewPanelItem = NewObject<URHStorePanelItem>();

			NewPanelItem->StoreItem = Item;
			NewPanelItem->HasBeenSeen = true;

			CurrentSectionItem->StorePanelItems.Add(NewPanelItem);
		}

		// Add new panel to the section
		if (CurrentSectionItem->StorePanelItems.Num())
		{
			CurrentSection->SectionItems.Add(CurrentSectionItem);
		}
	}

	return CurrentSection;
}

URHStoreItemHelper* URHStoreWidget::GetStoreItemHelper() const
{
    if (MyHud.IsValid())
    {
        return MyHud->GetItemHelper();
    }

    return nullptr;
}

bool URHStoreWidget::HasAllRequiredStoreInformation() const
{
	if (URHStoreItemHelper* StoreItemHelper = GetStoreItemHelper())
	{
		UE_LOG(RallyHereStart, Verbose, TEXT("URHStoreWidget::HasAllRequiredStoreInformation %s %s %s"), StoreItemHelper->StoreVendorsLoaded ? TEXT("true") : TEXT("false"),
																									   StoreItemHelper->ArePricePointsLoaded() ? TEXT("true") : TEXT("false"),
																									   StoreItemHelper->ArePortalOffersLoaded() ? TEXT("true") : TEXT("false"));
		return StoreItemHelper->StoreVendorsLoaded && StoreItemHelper->ArePricePointsLoaded() && StoreItemHelper->ArePortalOffersLoaded();
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHStoreWidget::HasAllRequiredStoreInformation failed to get Store Item Helper!"));
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////                                                                                                                                  ////
////                                                                                                                                  ////
////                                                          RHStoreSection                                                          ////
////                                                                                                                                  ////
////                                                                                                                                  ////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FText URHStoreSection::GetSectionHeader() const
{
    switch (SectionType)
    {
	case EStoreSectionTypes::AccountCosmetics:
		return NSLOCTEXT("RHStore", "FeaturedHeader", "FEATURED");
	case EStoreSectionTypes::Bundles:
		return NSLOCTEXT("RHStore", "Bundles", "BUNDLES");
	case EStoreSectionTypes::PortalOffers:
        	return NSLOCTEXT("RHStore", "DLCHeader", "SPECIAL PACKS");
    }

    return FText::FromString(TEXT("UnnamedSection"));
}

bool URHStoreSection::HasUnseenItems() const
{
	for (int32 i = 0; i < SectionItems.Num(); ++i)
	{
		if (SectionItems[i] && SectionItems[i]->HasUnseenItems())
		{
			return true;
		}
	}

	return false;
}

int32 URHStoreSection::GetSecondsRemaining() const
{
	FDateTime NowTime = FDateTime::UtcNow();

	if (NowTime <= SectionRotationEndTime)
	{
		FTimespan TimeRemaining = SectionRotationEndTime - NowTime;

		return TimeRemaining.GetTotalSeconds();
	}

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////                                                                                                                                  ////
////                                                                                                                                  ////
////                                                        RHStoreSectionItem                                                        ////
////                                                                                                                                  ////
////                                                                                                                                  ////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool URHStoreSectionItem::HasUnseenItems() const
{
	for (int32 i = 0; i < StorePanelItems.Num(); ++i)
	{
		if (StorePanelItems[i] && !StorePanelItems[i]->HasBeenSeen)
		{
			return true;
		}
	}

	return false;
}
