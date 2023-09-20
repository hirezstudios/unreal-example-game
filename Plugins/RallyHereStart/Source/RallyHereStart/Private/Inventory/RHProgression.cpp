// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "GameFramework/RHGameInstance.h"
#include "RH_CatalogSubsystem.h"
#include "Inventory/RHProgression.h"

URHProgression::URHProgression(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectInitializer)
{
	static const FGameplayTag ItemCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::ProgressionCollectionName);
	CollectionContainer.AddTag(ItemCollectionTag);
	
	IsOwnableInventoryItem = false;
	CanOwnMultiple = true;
}


int32 URHProgression::GetProgressionLevel(const class UObject* WorldContextObject, int32 ProgressAmount) const
{
	if (WorldContextObject == nullptr)
	{
		return -1;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (URHGameInstance* GameInstance = Cast<URHGameInstance>(World->GetGameInstance()))
		{
			if (URHStoreItemHelper* StoreItemHelper = GameInstance->GetStoreItemHelper())
			{
				FRHAPI_XpTable XpTable;
				if (StoreItemHelper->GetXpTable(ProgressionXpTableId, XpTable))
				{
					return URH_CatalogBlueprintLibrary::GetLevelAtXp(XpTable, ProgressAmount);
				}
			}
		}
	}

	return -1;
}

float URHProgression::GetProgressionLevelPercent(const class UObject* WorldContextObject, int32 ProgressAmount) const
{
	if (WorldContextObject == nullptr)
	{
		return 0.0f;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (URHGameInstance* GameInstance = Cast< URHGameInstance>(World->GetGameInstance()))
		{
			if (URHStoreItemHelper* StoreItemHelper = GameInstance->GetStoreItemHelper())
			{
				FRHAPI_XpTable XpTable;
				if (StoreItemHelper->GetXpTable(ProgressionXpTableId, XpTable))
				{
					float CurrentXpCount = ProgressAmount;
					int32 PlayerLevel = URH_CatalogBlueprintLibrary::GetLevelAtXp(XpTable, CurrentXpCount);

					float CurrentLevelXp = URH_CatalogBlueprintLibrary::GetXpAtLevel(XpTable, PlayerLevel);
					float NextLevelXp = URH_CatalogBlueprintLibrary::GetXpAtLevel(XpTable, PlayerLevel + 1);

					if ((NextLevelXp - CurrentLevelXp) >= 0)
					{
						return (CurrentXpCount - CurrentLevelXp) / (NextLevelXp - CurrentLevelXp);
					}
				}
			}
		}
	}

	return 0.0f;
}