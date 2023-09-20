// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "GameFramework/RHGameInstance.h"
#include "Inventory/RHLootBox.h"

URHLootBox::URHLootBox(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectInitializer)
{
	static const FGameplayTag ItemCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::LootBoxCollectionName);
	CollectionContainer.AddTag(ItemCollectionTag);
	
	IsOwnableInventoryItem = true;
}

void URHLootBox::CanOwnMore(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate)
{
	URHStoreItemHelper* StoreItemHelper = nullptr;
	
	if (UWorld* const World = GetWorld())
	{
		if (URHGameInstance* GameInstance = Cast<URHGameInstance>(World->GetGameInstance()))
		{
			StoreItemHelper = GameInstance->GetStoreItemHelper();
		}
	}

	if (StoreItemHelper == nullptr)
	{
		Delegate.ExecuteIfBound(false);
		return;
	}

	GetQuantityOwned(PlayerInfo, FRH_GetInventoryCountDelegate::CreateLambda([this, PlayerInfo, Delegate, StoreItemHelper](int32 ItemsToOwn)
		{
			TArray<URHStoreItem*> StoreItems = StoreItemHelper->GetStoreItemsForVendor(LootBoxVendorId, false, true);
			TArray<URHStoreItem*> StoreItemsToCheck;

			for (URHStoreItem* StoreItem : StoreItems)
			{
				if (StoreItem)
				{
					StoreItemsToCheck.Add(StoreItem);
				}
			}

			if (StoreItemsToCheck.Num())
			{
				bool HasResponsed = false;
				int32 Results = 0;

				for (auto& StoreItem : StoreItemsToCheck)
				{
					StoreItem->CanOwnMore(PlayerInfo, FRH_GetInventoryStateDelegate::CreateLambda([Delegate, StoreItemsToCheck, HasResponsed, ItemsToOwn, Results](bool bSuccess) mutable
						{
							Results++;

					if (bSuccess)
					{
						ItemsToOwn--;

						// If the player is missing more than the number of unopened loot boxes then we don't own it.
						if (ItemsToOwn < 0)
						{
							Delegate.ExecuteIfBound(false);
							HasResponsed = true;
						}
					}
					Results++;

					if (!HasResponsed && Results == StoreItemsToCheck.Num())
					{
						Delegate.ExecuteIfBound(true);
					}
						}));
				}
			}
			else
			{
				Delegate.ExecuteIfBound(true);
			}
		}));
}