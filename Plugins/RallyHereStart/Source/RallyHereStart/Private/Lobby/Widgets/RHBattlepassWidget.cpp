// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Subsystems/RHEventSubsystem.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "GameFramework/RHGameInstance.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/Widgets/RHPurchaseConfirmationWidget.h"
#include "Lobby/Widgets/RHBattlepassWidget.h"

void URHBattlepassWidget::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

void URHBattlepassWidget::UninitializeWidget_Implementation()
{
    Super::UninitializeWidget_Implementation();
}

void URHBattlepassWidget::OnShown_Implementation()
{
	Super::OnShown_Implementation();

	// Consume pending route data that may have been stored prior to this show
	UObject* PendingRouteData = nullptr;
	GetPendingRouteData(MyRouteTag, PendingRouteData);  //$$ KAB - Route names changed to Gameplay Tags
	SetPendingRouteData(MyRouteTag, nullptr);  //$$ KAB - Route names changed to Gameplay Tags

	// If we have a passed in battlepass data use that to display on screen
	DisplayedBattlepass = Cast<URHBattlepass>(PendingRouteData);
	
	// If we don't have passed in battlepass data, lets find the first active battlepass
	if (DisplayedBattlepass == nullptr)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (URHEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<URHEventSubsystem>())
			{
				DisplayedBattlepass = EventSubsystem->GetActiveEvent<URHBattlepass>();
			}
		}

		// If we don't have a displayed battlepass at this time, we should head back home
		if (DisplayedBattlepass == nullptr)
		{
			AddViewRoute(HomeViewTag); //$$ KAB - Route names changed to Gameplay Tags
			return;
		}
	}
}

void URHBattlepassWidget::ShowPurchaseBattlepass()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
		{
			if (URHPurchaseData* PurchaseData = NewObject<URHPurchaseData>())
			{
				PurchaseData->StoreItem = StoreSubsystem->GetStoreItem(DisplayedBattlepass->PassPurchaseLootId);

				if (PurchaseData->StoreItem != nullptr)
				{
					// If we are going to go to the purchase confirmation screen for an item not in memory, start the load now
					if (!PurchaseData->StoreItem->GetInventoryItem().IsValid())
					{
						UAssetManager::GetStreamableManager().RequestAsyncLoad(PurchaseData->StoreItem->GetInventoryItem().ToSoftObjectPath());
					}
					
					PurchaseData->ExternalTransactionId = "PurchaseBattlepass";
					AddViewRoute(PurchaseConfirmationViewTag, false, false, PurchaseData); //$$ KAB - Route names changed to Gameplay Tags
				}
			}
		}
	}
}

void URHBattlepassWidget::ShowPurchaseBattlepassTiers(int32 TierCount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
		{
			if (URHStoreItemWithBattlepassData* PurchaseData = NewObject<URHStoreItemWithBattlepassData>())
			{
				PurchaseData->StoreItem = StoreSubsystem->GetStoreItem(DisplayedBattlepass->LevelPurchaseLootId);

				if (PurchaseData->StoreItem != nullptr)
				{
					// If we are going to go to the purchase confirmation screen for an item not in memory, start the load now
					if (!PurchaseData->StoreItem->GetInventoryItem().IsValid())
					{
						UAssetManager::GetStreamableManager().RequestAsyncLoad(PurchaseData->StoreItem->GetInventoryItem().ToSoftObjectPath());
					}

					PurchaseData->BattlepassItem = DisplayedBattlepass;
					PurchaseData->PurchaseQuantity = TierCount;
					PurchaseData->ExternalTransactionId = "PurchaseBattlepass";
					AddViewRoute(PurchaseConfirmationViewTag, false, false, PurchaseData); //$$ KAB - Route names changed to Gameplay Tags
				}
			}
		}
	}
}