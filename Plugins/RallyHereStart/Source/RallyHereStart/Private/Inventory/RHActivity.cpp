// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "GameFramework/RHGameInstance.h"
#include "RH_GameInstanceSubsystem.h"
#include "Inventory/RHActivity.h"

URHActivity::URHActivity(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	static const FGameplayTag ActivityCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::ActivityCollectionName);
	CollectionContainer.AddTag(ActivityCollectionTag);
}

void URHActivity::IsActive(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate)
{
	if (RequiredItemId.IsValid() && PlayerInfo != nullptr)
	{
		if (URH_PlayerInventory* PlayerInventory = PlayerInfo->GetPlayerInventory())
		{
			PlayerInventory->GetInventoryCount(RequiredItemId, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [Delegate](int32 Count)
				{
					Delegate.ExecuteIfBound(Count > 0);
				}));
			return;
		}
	}
	else
	{
		// By default if there is no RequiredItem, then the Activity will be active
		Delegate.ExecuteIfBound(true);
		return;
	}

	Delegate.ExecuteIfBound(false);
}

void URHActivity::GetCurrentProgress(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate)
{
	if (GetItemId().IsValid() && PlayerInfo != nullptr)
	{
		if (URH_PlayerInventory* PlayerInventory = PlayerInfo->GetPlayerInventory())
		{
			PlayerInventory->GetInventoryCount(GetItemId(), Delegate);
			return;
		}
	}

	Delegate.ExecuteIfBound(0);
}

int32 URHActivity::GetCompletionProgressCount()
{
	return 0;
}

void URHActivity::HasBeenCompleted(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate)
{
	GetCurrentProgress(PlayerInfo, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [this, Delegate](int32 Count)
		{
			Delegate.ExecuteIfBound(Count >= GetCompletionProgressCount());
		}));
}

bool URHActivity::AddLootReward(URHGameInstance* GameInstance, class URH_PlayerInfo* PlayerInfo, int32 Count) const
{
	if (GameInstance != nullptr && PlayerInfo != nullptr && ProgressLootId.IsValid() && Count > 0)
	{
		TArray<URH_PlayerOrderEntry*> PlayerOrderEntries;

		URH_PlayerOrderEntry* NewPlayerOrderEntry = NewObject<URH_PlayerOrderEntry>();

		NewPlayerOrderEntry->FillType = ERHAPI_PlayerOrderEntryType::FillLoot;
		NewPlayerOrderEntry->LootId = ProgressLootId;
		NewPlayerOrderEntry->Quantity = Count;
		NewPlayerOrderEntry->ExternalTransactionId = "Instance Activity Progress";

		PlayerOrderEntries.Push(NewPlayerOrderEntry);

		PlayerInfo->GetPlayerInventory()->CreateNewPlayerOrder(ERHAPI_Source::Instance, false, PlayerOrderEntries);
		UE_LOG(LogActivity, Log, TEXT("URHActivity::AddLootReward -- Added Activity Loot Reward -- PlayerId=%s, ItemId=%s, LootId=%s, Count=%d"), *PlayerInfo->GetRHPlayerUuid().ToString(), *GetItemId().ToString(), *ProgressLootId.ToString(), Count);
		return true;
	}
	
	return false;
}