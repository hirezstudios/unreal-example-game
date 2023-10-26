// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Subsystems/RHStoreSubsystem.h"
#include "GameFramework/RHGameInstance.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RH_CatalogSubsystem.h"
#include "Inventory/RHBattlepass.h"

URHBattlepass::URHBattlepass(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	static const FGameplayTag BattlepassCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::BattlepassCollectionName);
	CollectionContainer.AddTag(BattlepassCollectionTag);

	FreeIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("FreeIconInfo"));
	PremiumIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("PremiumIconInfo"));
	RewardsFullyLoaded = false;
	InstantUnlocksFullyLoaded = false;
}

void URHBattlepass::AppendRequiredLootTableIds(TArray<int32>& LootTableIds)
{
	Super::AppendRequiredLootTableIds(LootTableIds);

	LootTableIds.Push(InstantUnlockRewardVendorId);
	LootTableIds.Push(FreeRewardVendorId);
	LootTableIds.Push(PremiumRewardVendorId);
	LootTableIds.Push(PurchaseVendorId);
}

void URHBattlepass::GetTotalXpProgress(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate)
{
	if (ProgressItemId.IsValid() && PlayerInfo != nullptr)
	{
		if (URH_PlayerInventory* PlayerInventory = PlayerInfo->GetPlayerInventory())
		{
			PlayerInventory->GetInventoryCount(ProgressItemId, Delegate);
			return;
		}
	}
	Delegate.ExecuteIfBound(0);
}

void URHBattlepass::GetCurrentLevel(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate)
{
	if (ProgressItemId.IsValid() && PlayerInfo != nullptr)
	{
		if (UWorld* World = PlayerInfo->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
				{
					FRHAPI_XpTable XpTable;
					if (StoreSubsystem->GetXpTable(XpTableId, XpTable))
					{
						if (URH_PlayerInventory* PlayerInventory = PlayerInfo->GetPlayerInventory())
						{
							PlayerInventory->GetInventoryCount(ProgressItemId, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [XpTable, Delegate](int32 Count)
								{
									Delegate.ExecuteIfBound(URH_CatalogBlueprintLibrary::GetLevelAtXp(XpTable, Count));
								}));
							return;
						}
					}
				}
			}
		}
	}
	Delegate.ExecuteIfBound(0);
}

int32 URHBattlepass::GetLevelCount(const UObject* WorldContextObject) const
{
	if (ProgressItemId.IsValid())
	{
		UWorld* const World = GEngine != nullptr ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;

		if (World != nullptr)
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
				{
					FRHAPI_XpTable XpTable;
					if (StoreSubsystem->GetXpTable(XpTableId, XpTable))
					{
						if (const auto& Entries = XpTable.GetXpEntriesOrNull())
						{
							return (*Entries).Num();
						}
					}
				}
			}
		}
	}

	return 0;
}

TArray<URHBattlepassLevel*> URHBattlepass::GetLevels(const UObject* WorldContextObject)
{
	// If we don't have cached levels, generate our level info now
	if (!CachedBattlepassLevels.Num())
	{
		if (URHStoreSubsystem* StoreSubsystem = GetStoreSubsystem(WorldContextObject))
		{
			// First Create the Levels from the XP table
			FRHAPI_XpTable XpTable;
			if (StoreSubsystem->GetXpTable(XpTableId, XpTable))
			{
				if (const auto& Entries = XpTable.GetXpEntriesOrNull())
				{
					int32 i = 0;
					for (const auto& Entry : *Entries)
					{
						if (URHBattlepassLevel* NewLevel = NewObject<URHBattlepassLevel>())
						{
							// We index our XP levels where the player starts at 0 and earns XP to become level 1
							NewLevel->LevelNumber = i + 1;
							NewLevel->StartXp = i > 0 ? URH_CatalogBlueprintLibrary::GetXpAtLevel(XpTable, i - 1) : 0, URH_CatalogBlueprintLibrary::GetXpAtLevel(XpTable, i);
							NewLevel->EndXp = URH_CatalogBlueprintLibrary::GetXpAtLevel(XpTable, i);
							CachedBattlepassLevels.Push(NewLevel);
						}
						i++;
					}
				}
			}

			// Then fill out the rewards
			auto FillOutRewards = [this, StoreSubsystem](int32 VendorId, EBattlepassTrackType TrackType)
			{
				if (VendorId > 0)
				{
					TArray<URHStoreItem*> StoreItems = StoreSubsystem->GetStoreItemsForVendor(VendorId, false, false);

					TArray<FSoftObjectPath> AssetsToLoad;

					for (URHStoreItem* StoreItem : StoreItems)
					{
						int32 LevelNumber = StoreItem->GetSortOrder();

						if (!StoreItem->GetInventoryItem().IsValid())
						{
							AssetsToLoad.Add(StoreItem->GetInventoryItem().ToSoftObjectPath());
						}

						if (LevelNumber > 0)
						{
							if (URHBattlepassLevel* Level = GetBattlepassLevel(LevelNumber))
							{
								if (URHBattlepassRewardItem* NewReward = NewObject<URHBattlepassRewardItem>())
								{
									NewReward->Item = StoreItem;
									NewReward->Track = TrackType;
									Level->RewardItems.Push(NewReward);
								}
							}
						}
					}

					if (AssetsToLoad.Num() > 0)
					{
						UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateUObject(this, &URHBattlepass::RewardAssetsFullyLoaded));
					}
					else
					{
						RewardAssetsFullyLoaded();
					}
				}
			};

			FillOutRewards(FreeRewardVendorId, EBattlepassTrackType::EFreeTrack);
			FillOutRewards(PremiumRewardVendorId, EBattlepassTrackType::EPremiumTrack);
		}
	}

	return CachedBattlepassLevels;
}

void URHBattlepass::RewardAssetsFullyLoaded()
{
	RewardsFullyLoaded = true;
}

void URHBattlepass::InstantUnlocksAssetsFullyLoaded()
{
	InstantUnlocksFullyLoaded = true;
}

TArray<URHBattlepassRewardItem*> URHBattlepass::GetInstantUnlockRewards(const UObject* WorldContextObject)
{
	// If we don't have cached instant unlocks, generate it now
	if (!CachedInstantUnlocks.Num() && InstantUnlockRewardVendorId > 0)
	{
		if (URHStoreSubsystem* StoreSubsystem = GetStoreSubsystem(WorldContextObject))
		{
			TArray<URHStoreItem*> StoreItems = StoreSubsystem->GetStoreItemsForVendor(InstantUnlockRewardVendorId, false, false);

			for (URHStoreItem* StoreItem : StoreItems)
			{
				TArray<FSoftObjectPath> AssetsToLoad;

				if (URHBattlepassRewardItem* NewReward = NewObject<URHBattlepassRewardItem>())
				{
					NewReward->Item = StoreItem;
					NewReward->Track = EBattlepassTrackType::EInstantUnlock;
					CachedInstantUnlocks.Push(NewReward);
				}

				if (!StoreItem->GetInventoryItem().IsValid())
				{
					AssetsToLoad.Add(StoreItem->GetInventoryItem().ToSoftObjectPath());
				}

				if (AssetsToLoad.Num() > 0)
				{
					UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateUObject(this, &URHBattlepass::InstantUnlocksAssetsFullyLoaded));
				}
				else
				{
					InstantUnlocksAssetsFullyLoaded();
				}
			}
		}
	}

	return CachedInstantUnlocks;
}

URHBattlepassLevel* URHBattlepass::GetBattlepassLevel(int32 LevelNumber) const
{
	for (URHBattlepassLevel* Level : CachedBattlepassLevels)
	{
		if (Level->LevelNumber == LevelNumber)
		{
			return Level;
		}
	}

	return nullptr;
}

URHStoreSubsystem* URHBattlepass::GetStoreSubsystem(const UObject* WorldContextObject)
{
	UWorld* const World = GEngine != nullptr ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;

	if (World != nullptr)
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<URHStoreSubsystem>();
		}
	}

	return nullptr;
}

#if WITH_EDITOR
void URHBattlepass::GetPreloadDependencies(TArray<UObject*>& OutDeps)
{
	Super::GetPreloadDependencies(OutDeps);

	if (FreeIconInfo != nullptr)
	{
		OutDeps.Add(FreeIconInfo);
	}

	if (PremiumIconInfo != nullptr)
	{
		OutDeps.Add(PremiumIconInfo);
	}
}
#endif
