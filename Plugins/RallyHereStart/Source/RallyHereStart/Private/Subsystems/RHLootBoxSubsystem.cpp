#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RH_CatalogSubsystem.h"
#include "RH_GameInstanceSubsystem.h"
#include "GameFramework/RHGameInstance.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Subsystems/RHLootBoxSubsystem.h"

bool URHLootBoxSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}

void URHLootBoxSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	StoreSubsystem = Collection.InitializeDependency<URHStoreSubsystem>();

	Super::Initialize(Collection);

	HasRequestedVendors = false;

	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		GameInstance->OnLocalPlayerLoginChanged.AddUObject(this, &URHLootBoxSubsystem::OnLoginPlayerChanged);
	}
}

void URHLootBoxSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		GameInstance->OnLocalPlayerLoginChanged.RemoveAll(this);
	}
}

void URHLootBoxSubsystem::OnLoginPlayerChanged(ULocalPlayer* LocalPlayer)
{
	if (StoreSubsystem && !HasRequestedVendors)
	{
		HasRequestedVendors = true;

		if (LootBoxsVendorId > 0 && LootBoxsRedemptionVendorId > 0)
		{
			TArray<int32> VendorIds;

			VendorIds.Push(LootBoxsVendorId);
			VendorIds.Push(LootBoxsRedemptionVendorId);

			StoreSubsystem->RequestVendorData(VendorIds, FRH_CatalogCallDelegate::CreateUObject(this, &URHLootBoxSubsystem::OnStoreVendorsLoaded));
		}
	}
}

void URHLootBoxSubsystem::OnStoreVendorsLoaded(bool bSuccess)
{
	if (StoreSubsystem)
	{
		// Fill out a quick lookup of loot ids that are from loot boxes for efficient order handling in future
		LootBoxLootIds.Empty();
		LootBoxLootIds.Append(StoreSubsystem->GetLootIdsForVendor(LootBoxsRedemptionVendorId, false, true));

		// Fill out a contents table of unopened loot boxes for quick grabbing if multiple screens may need this info
		UnopenedLootBoxIdToContents.Empty();

		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (auto pGISubsystem = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
			{
				if (URH_CatalogSubsystem* CatalogSubsystem = pGISubsystem->GetCatalogSubsystem())
				{
					TArray<URHStoreItem*> LootBoxs = StoreSubsystem->GetStoreItemsForVendor(LootBoxsVendorId, false, false);

					FRHAPI_Vendor RedemptionVendor;
					if (CatalogSubsystem->GetVendorById(LootBoxsRedemptionVendorId, RedemptionVendor))
					{
						for (URHStoreItem* LootBox : LootBoxs)
						{
							if (URHLootBox* LootBoxItem = Cast<URHLootBox>(LootBox->GetInventoryItem().Get()))
							{
								if (URHLootBoxDetails* DropDetails = NewObject<URHLootBoxDetails>())
								{
									DropDetails->LootBox = LootBox;
									FRHAPI_Loot LootBoxTableItem;
									if (URH_CatalogBlueprintLibrary::GetVendorItemById(RedemptionVendor, LootBoxItem->LootBoxVendorId, LootBoxTableItem))
									{
										const auto* SubVendorId = LootBoxTableItem.GetSubVendorIdOrNull();

										if (SubVendorId == nullptr)
										{
											continue;
										}

										FRHAPI_Vendor SubVendor;
										if (CatalogSubsystem->GetVendorById(*SubVendorId, SubVendor))
										{
											if (const auto& LootItems = SubVendor.GetLootOrNull())
											{
												for (const auto& pItemPair : (*LootItems))
												{
													FRHAPI_Loot SubVendorItem = pItemPair.Value;

													const auto* ContentsVendorId = SubVendorItem.GetSubVendorIdOrNull();
													const auto* IsActive = SubVendorItem.GetActiveOrNull();

													if (ContentsVendorId == nullptr || IsActive == nullptr || !*IsActive)
													{
														continue;
													}

													if (URHLootBoxContents* Contents = NewObject<URHLootBoxContents>())
													{
														Contents->LootTableId = *ContentsVendorId;
														Contents->BundleContents.Append(StoreSubsystem->GetStoreItemsForVendor(Contents->LootTableId, false, false));

														for (TSoftObjectPtr<URHStoreItem> ContentItem : Contents->BundleContents)
														{
															ELootBoxContentsCategories TargetCategory = GetSubCategoryForItem(ContentItem.Get());

															TArray<TSoftObjectPtr<URHStoreItem>>* pContentsArray = Contents->ContentsBySubcategory.Find(TargetCategory);
															if (pContentsArray)
															{
																pContentsArray->Add(ContentItem);
															}
															else
															{
																TArray<TSoftObjectPtr<URHStoreItem>> NewContentCategory;
																NewContentCategory.Add(ContentItem);
																Contents->ContentsBySubcategory.Add(TargetCategory, NewContentCategory);
															}
														}

														DropDetails->Contents.Add(Contents);
													}
												}
											}

											UnopenedLootBoxIdToContents.Add(LootBox->GetLootId(), DropDetails);
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
}

ELootBoxContentsCategories URHLootBoxSubsystem::GetSubCategoryForItem(URHStoreItem* StoreItem) const
{
	static const FGameplayTag AvatarCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::AvatarCollectionName);
	static const FGameplayTag BannerCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::BannerCollectionName);
	static const FGameplayTag BorderCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::BorderCollectionName);
	static const FGameplayTag TitleCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::TitleCollectionName);

	if (StoreItem && StoreItem->GetInventoryItem().IsValid())
	{
		if (UPlatformInventoryItem* Item = StoreItem->GetInventoryItem().Get())
		{
			if (Item->GetCollectionContainer().HasTag(AvatarCollectionTag))
			{
				return ELootBoxContentsCategories::LootBoxContents_Avatars;
			}

			if (Item->GetCollectionContainer().HasTag(BannerCollectionTag))
			{
				return ELootBoxContentsCategories::LootBoxContents_Banners;
			}

			if (Item->GetCollectionContainer().HasTag(TitleCollectionTag))
			{
				return ELootBoxContentsCategories::LootBoxContents_Titles;
			}

			if (Item->GetCollectionContainer().HasTag(BorderCollectionTag))
			{
				return ELootBoxContentsCategories::LootBoxContents_Borders;
			}
		}
	}

	return ELootBoxContentsCategories::LootBoxContents_Other;
}

FText URHLootBoxSubsystem::GetContentCategoryName(ELootBoxContentsCategories Category)
{
	switch (Category)
	{
		case ELootBoxContentsCategories::LootBoxContents_All:
			return NSLOCTEXT("RHItemType", "All", "All");
		case ELootBoxContentsCategories::LootBoxContents_Avatars:
			return NSLOCTEXT("RHItemType", "AvatarPlural", "Avatars");
		case ELootBoxContentsCategories::LootBoxContents_Banners:
			return NSLOCTEXT("RHItemType", "BannerPlural", "Banners");
		case ELootBoxContentsCategories::LootBoxContents_Titles:
			return NSLOCTEXT("RHItemType", "TitlePlural", "Titles");
		case ELootBoxContentsCategories::LootBoxContents_Borders:
			return NSLOCTEXT("RHItemType", "BorderPlural", "Borders");
		case ELootBoxContentsCategories::LootBoxContents_Other:
			return FText::FromString(TEXT("UNCATEGORIZED, FIX BEFORE DEPLOY"));
	}

	return FText::FromString("");
}

URHLootBoxDetails* URHLootBoxSubsystem::GetLootBoxDetails(URHStoreItem* LootBox)
{
	if (LootBox)
	{
		return *UnopenedLootBoxIdToContents.Find(LootBox->GetLootId());
	}

	return nullptr;
}

void URHLootBoxSubsystem::CallOnLootBoxOpenStarted()
{
	OnLootBoxOpenStarted.Broadcast();
}

void URHLootBoxSubsystem::CallOnLootBoxOpenFailed()
{
	OnLootBoxOpenFailed.Broadcast();
}

void URHLootBoxSubsystem::CallOnLootBoxContentsReceived(TArray<UPlatformInventoryItem*>& Items)
{
	OnLootBoxContentsReceived.Broadcast(Items);
}

void URHLootBoxSubsystem::CallOnDisplayLootBoxIntro(URHLootBox* LootBox)
{
	OnDisplayLootBoxIntro.Broadcast(LootBox);
}

void URHLootBoxSubsystem::CallOnDisplayLootBoxIntroAndOpen(URHLootBox* LootBox)
{
	OnDisplayLootBoxIntroAndOpen.Broadcast(LootBox);
}

void URHLootBoxSubsystem::CallOnLootBoxLeave()
{
	OnLootBoxLeave.Broadcast();
}

void URHLootBoxSubsystem::CallOnLootBoxOpeningSequenceComplete()
{
	OnLootBoxOpenSequenceCompleted.Broadcast();
}

bool URHLootBoxSubsystem::IsFromLootBox(URHStoreItem* StoreItem)
{
	return StoreItem && LootBoxLootIds.Contains(StoreItem->GetLootId());
}

TArray<FText> URHLootBoxContents::GetContentsFilterOptions()
{
	TArray<FText> Options;

	FString CategoryName = URHLootBoxSubsystem::GetContentCategoryName(ELootBoxContentsCategories::LootBoxContents_All).ToString();
	int32 Count = BundleContents.Num();

	Options.Add(FText::FromString(FString::Printf(L"%s (%d)", *CategoryName, Count)));

	for (int32 i = 0; i <= ELootBoxContentsCategories::LootBoxContents_Other; ++i)
	{
		TArray<TSoftObjectPtr<URHStoreItem>>* ContentsArray = ContentsBySubcategory.Find(ELootBoxContentsCategories(i));

		if (ContentsArray)
		{
			CategoryName = URHLootBoxSubsystem::GetContentCategoryName(ELootBoxContentsCategories(i)).ToString();
			Count = ContentsArray->Num();
			Options.Add(FText::FromString(FString::Printf(L"%s (%d)", *CategoryName, Count)));
		}
	}

	return Options;
}

TArray<TSoftObjectPtr<URHStoreItem>> URHLootBoxContents::GetContentsByFilterIndex(int32 Index)
{
	// If we are the first index return the full contents array instead
	if (Index == 0)
	{
		return BundleContents;
	}
	else
	{
		int32 FoundCategories = 0;

		for (int32 i = 1; i <= ELootBoxContentsCategories::LootBoxContents_Other; ++i)
		{
			TArray<TSoftObjectPtr<URHStoreItem>>* ContentsArray = ContentsBySubcategory.Find(ELootBoxContentsCategories(i));

			if (ContentsArray)
			{
				FoundCategories++;

				if (FoundCategories == Index)
				{
					return *ContentsArray;
				}
			}
		}
	}

	TArray<TSoftObjectPtr<URHStoreItem>> Contents;
	return Contents;
}
