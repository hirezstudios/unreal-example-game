#include "RallyHereStart.h"
#include "GameFramework/RHGameInstance.h"
#include "GameFramework/RHGameUserSettings.h"
#include "Inventory/RHCurrency.h"
#include "Managers/RHViewManager.h"
#include "Subsystems/RHLoadoutSubsystem.h"
#include "Subsystems/RHLootBoxSubsystem.h"
#include "Subsystems/RHOrderSubsystem.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RH_GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"

DECLARE_STATS_GROUP(TEXT("RallyHereStart"), STATGROUP_RallyHereStart, STATCAT_Advanced);

URHLootBoxSubsystem* GetLootBoxManager(const UObject* WorldContextObject)
{
	if (WorldContextObject != nullptr)
	{
		if (UWorld* const World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				return GameInstance->GetSubsystem<URHLootBoxSubsystem>();
			}
		}
	}

	return nullptr;
}

URHLoadoutSubsystem* GetLoadoutSubsystem(const UObject* WorldContextObject)
{
	if (WorldContextObject != nullptr)
	{
		if (UWorld* const World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				return GameInstance->GetSubsystem<URHLoadoutSubsystem>();
			}
		}
	}

	return nullptr;
}

bool URHOrderSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}

void URHOrderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	StoreSubsystem = Collection.InitializeDependency<URHStoreSubsystem>();
	Super::Initialize(Collection);

	UE_LOG(RallyHereStart, Log, TEXT("URHOrderSubsystem::Initialize()."));

	if (StoreSubsystem != nullptr)
	{
		StoreSubsystem->OnPendingPurchaseReceivedNative.BindUObject(this, &URHOrderSubsystem::OnPendingPurchaseReceived);
	}
}

TStatId URHOrderSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URHOrderSubsystem, STATGROUP_RallyHereStart);
}

void URHOrderSubsystem::Tick(float DeltaTime)
{
    // Process the Pending Order and promote any to Queued as Needed
    if (PendingOrder)
    {
        PendingOrder->TimeSinceCreation += DeltaTime;

        // Any pending order over a small time old is ready to promote
        if (PendingOrder->TimeSinceCreation > 0.5f)
        {
            QueuedOrders.Add(PendingOrder);
            PendingOrder = nullptr;

			// If the Queue was empty before we added this order check if we can display it now
			if (QueuedOrders.Num() == 1)
			{
				DisplayNextOrder();
			}
        }
    }
}

URHOrder* URHOrderSubsystem::GetNextOrder()
{
    URHOrder* NextOrder = nullptr;

    if (QueuedOrders.Num() > 0)
    {
        NextOrder = QueuedOrders[0];
        QueuedOrders.RemoveAt(0);
    }

    return NextOrder;
}

void URHOrderSubsystem::DisplayNextOrder()
{
    if (QueuedOrders.Num() > 0 && CanDisplayOrder())
    {
        // Fire off and remove our next order
        OnOrderReady.Broadcast(QueuedOrders[0]);
        QueuedOrders.RemoveAt(0);
    }
}

void URHOrderSubsystem::SetOrderWatchForPlayer(URH_PlayerInfo* PlayerInfo, const FDateTime& FirstValidTime)
{
	// If we are already watching the player ignore the request
	if (PlayerInfo != nullptr)
	{
		FRH_ActiveOrderWatch* OrderWatch = ActiveOrderWatches.FindByPredicate([PlayerInfo](const FRH_ActiveOrderWatch& Watch)
			{
				return Watch.PlayerInfo == PlayerInfo;
			});

		if (OrderWatch != nullptr)
		{
			return;
		}
	
		const URH_PlayerInfo* ConstPlayerInfo = PlayerInfo;

		FRH_ActiveOrderWatch NewWatch = FRH_ActiveOrderWatch();
		NewWatch.PlayerInfo = PlayerInfo;
		NewWatch.OrderDetailsBlock = FRH_OrderDetailsDelegate::CreateUObject(this, &URHOrderSubsystem::OnPlayerOrder, ConstPlayerInfo);
		NewWatch.FirstValidOrderTime = FirstValidTime;
		PlayerInfo->GetPlayerInventory()->SetOrderWatch(NewWatch.OrderDetailsBlock);
		ActiveOrderWatches.Add(NewWatch);
	}
}

void URHOrderSubsystem::ClearOrderWatchForPlayer(URH_PlayerInfo* PlayerInfo)
{
	if (PlayerInfo != nullptr)
	{
		for (int32 i = 0; i < ActiveOrderWatches.Num(); ++i)
		{
			if (ActiveOrderWatches[i].PlayerInfo == PlayerInfo)
			{
				PlayerInfo->GetPlayerInventory()->ClearOrderWatch(ActiveOrderWatches[i].OrderDetailsBlock);
				ActiveOrderWatches.RemoveAt(i);
				return;
			}
		}
	}
}

void URHOrderSubsystem::OnPendingPurchaseReceived(const URH_PlayerInfo* PlayerInfo, const FRHAPI_PlayerOrder& PlayerOrderResult)
{
	FRH_ActiveOrderWatch* OrderWatch = ActiveOrderWatches.FindByPredicate([PlayerInfo](const FRH_ActiveOrderWatch& Watch)
	{
		return Watch.PlayerInfo == PlayerInfo;
	});

	if (OrderWatch != nullptr)
	{
		TArray<FRHAPI_PlayerOrder> Orders;
		Orders.Push(PlayerOrderResult);
		OnPlayerOrder(Orders, PlayerInfo);
	}
}

void URHOrderSubsystem::OnPlayerOrder(const TArray<FRHAPI_PlayerOrder>& OrderResults, const URH_PlayerInfo* PlayerInfo)
{
	if (StoreSubsystem == nullptr)
	{
		return;
	}

	URH_CatalogSubsystem* CatalogSubsystem = nullptr;
	URHGameUserSettings* pRHGameSettings = Cast<URHGameUserSettings>(GEngine->GetGameUserSettings());
	bool bSaveSettings = false;

	if (UWorld* const World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (auto pGISubsystem = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
			{
				CatalogSubsystem = pGISubsystem->GetCatalogSubsystem();
			}
		}
	}

	if (CatalogSubsystem == nullptr)
	{
		return;
	}

	FRH_ActiveOrderWatch* OrderWatch = ActiveOrderWatches.FindByPredicate([PlayerInfo](const FRH_ActiveOrderWatch& Watch)
		{
			return Watch.PlayerInfo == PlayerInfo;
		});

	// If we no longer are watching for this player, we should probably not process it
	if (OrderWatch == nullptr)
	{
		return;
	}

	for (const auto& OrderResult : OrderResults)
	{
		// If an order is before the time we want to check for orders, ignore it.
		if (OrderResult.GetCreatedTime() < OrderWatch->FirstValidOrderTime)
		{
			continue;
		}

		if (OrderWatch->SeenOrderIds.Contains(OrderResult.GetOrderId()))
		{
			continue;
		}

		OrderWatch->SeenOrderIds.Add(OrderResult.GetOrderId());

		if (const auto Source = OrderResult.GetSourceOrNull())
		{
			switch (*Source)
			{
			case ERHAPI_Source::LoginGrant:
			case ERHAPI_Source::Entitlements:
				continue; // ignore orders from these sources
			}
		}

		for (const auto& OrderEntry : OrderResult.GetEntries())
		{	
			if (const auto Result = OrderEntry.GetResultOrNull())
			{
				switch (*Result)
				{
					case ERHAPI_PlayerOrderEntryResult::Success:
						break;
					case ERHAPI_PlayerOrderEntryResult::InternalError:
					case ERHAPI_PlayerOrderEntryResult::FailedToFindAnySubLoot:
					case ERHAPI_PlayerOrderEntryResult::FailedToFillAnySubLoot:
					case ERHAPI_PlayerOrderEntryResult::CannotMeetLootBlocker:
					case ERHAPI_PlayerOrderEntryResult::CannotMeetLootRequired:
					case ERHAPI_PlayerOrderEntryResult::FailedToConsumeLootRequired:
					case ERHAPI_PlayerOrderEntryResult::FailedToSubmitNewOrder:
					case ERHAPI_PlayerOrderEntryResult::FailedToModifyInventory:
						continue; // Internal errors happen various weird times, we want to ignore those for giving failures for now.
					case ERHAPI_PlayerOrderEntryResult::TransientSingleLootAlreadyApplied:
						continue; // Error regarding something having previously been successful, so we don't want to display anything to the user.
					default:
						OnOrderFailed.Broadcast(NSLOCTEXT("OrderSubsystem", "Error", "An error occured while processing your order"));
						continue;
				}
			}
			else
			{
				OnOrderFailed.Broadcast(NSLOCTEXT("OrderSubsystem", "Error", "An error occured while processing your order"));
				continue;
			}

			auto ProcessOrderForItem = [this, OrderEntry, OrderResult, pRHGameSettings, &bSaveSettings, PlayerInfo](const FRHAPI_Loot& LootItem, TOptional<ERHAPI_InventoryBucket> Bucket = TOptional<ERHAPI_InventoryBucket>()) mutable
			{
				int32 quantity = OrderEntry.GetQuantity();

				if (const auto& SelectorType = LootItem.GetInventorySelectorTypeOrNull())
				{
					if (*SelectorType == ERHAPI_InventorySelector::OwnTransient)
					{
						if (pRHGameSettings->GetSavedTransientOrderIds().Contains(FRH_LootId(LootItem.GetLootId())))
						{
							return;
						}

						bSaveSettings |= pRHGameSettings->SaveTransientOrderId(LootItem.GetLootId());
					}
				}

				if (const auto& ItemId = LootItem.GetItemIdOrNull())
				{
					if (const auto PurchasePrice = OrderEntry.GetPurchasePriceOrNull())
					{
						if (*ItemId > 0 && (*ItemId == PurchasePrice->GetPriceItemId() || *ItemId == PurchasePrice->GetPriceCouponItemId()))
						{
							return; // don't show orders for the currency spent on the purchase
						}
					}
				}
				else
				{
					return;
				}

				// Don't create orders unless it is for an item being granted
				if (const auto& InventoryOperation = LootItem.GetInventoryOperationOrNull())
				{
					if (*InventoryOperation != ERHAPI_InventoryOperation::Add && *InventoryOperation != ERHAPI_InventoryOperation::Set)
					{
						return;
					}
				}
				else
				{
					return;
				}

				CreateOrderForItem(
					StoreSubsystem->GetStoreItemForVendor(LootItem.GetVendorId(), LootItem.GetLootId()),
					PlayerInfo,
					quantity,
					OrderResult.GetClientOrderRefId(FGuid()),
					Bucket);
			};

			if (const auto Details = OrderEntry.GetDetailsOrNull())
			{
				for (const auto& Detail : *Details)
				{
					if (Detail.GetType() == ERHAPI_PlayerOrderDetailType::InventoryChange)
					{
						TOptional<ERHAPI_InventoryBucket> InvBucket;
						if (const auto& InvChange = Detail.GetInvChangeOrNull())
						{
							if (const auto& After = (*InvChange).GetAfterOrNull())
							{
								if (const auto& Bucket = (*After).GetBucketOrNull())
								{
									InvBucket = *Bucket;
								}
							}
						}

						if (const auto LootId = Detail.GetLootIdOrNull())
						{
							FRHAPI_Loot LootItem;
							if (CatalogSubsystem->GetVendorItemByLootId(*LootId, LootItem))
							{
								ProcessOrderForItem(LootItem, InvBucket);
							}
						}
					}
				}
			}
			else if (const auto LootId = OrderEntry.GetLootIdOrNull())
			{
				FRHAPI_Loot LootItem;
				if (CatalogSubsystem->GetVendorItemByLootId(*LootId, LootItem))
				{
					ProcessOrderForItem(LootItem);
				}
			}
		}
	}

	if (pRHGameSettings && bSaveSettings)
	{
		pRHGameSettings->SaveSettings();
	}
}

void URHOrderSubsystem::CreateOrderForItem(URHStoreItem* StoreItem, const URH_PlayerInfo* PlayerInfo)
{
	CreateOrderForItem(StoreItem, PlayerInfo, 1, FGuid{}, TOptional<ERHAPI_InventoryBucket>()); // Blueprint doesn't like default Guid parameters
}

void URHOrderSubsystem::CreateOrderForItem(URHStoreItem* StoreItem, const URH_PlayerInfo* PlayerInfo, int32 Quantity, const FGuid& PurchaseRefId, TOptional<ERHAPI_InventoryBucket> eInventoryBucket)
{
    if (!StoreItem)
    {
		return;
    }

	// If the order is not for an actual item, or we can't get a reference to the item, we won't display an order for it because we don't know what it is
	TSoftObjectPtr<UPlatformInventoryItem> InventoryItem = StoreItem->GetInventoryItem();
	if (!InventoryItem.IsValid() && !InventoryItem.IsNull())
	{
		InventoryItem.LoadSynchronous();
	}

	if (!InventoryItem.IsValid())
	{
		return;
	}

	if (!InventoryItem->ShouldDisplayItemOrder(StoreItem->GetLootId()))
	{
		return;
	}

	// Check if this order is part of the pending items for a given bundle
	URH_CatalogItem* pItem = StoreItem->GetCatalogItem();
	if (!pItem)
	{
		return;
	}

	ERHOrderType OrderType = GetItemOrderType(StoreItem);

	if (GetItemOrderType(StoreItem) == ERHOrderType::Voucher)
	{
		if (!StoreSubsystem)
		{
			return;
		}

		if (!StoreSubsystem->CanRedeemVoucher(StoreItem))
		{
			OnInvalidVoucherObtained.Broadcast(StoreItem);
			return;
		}
	}

	if (eInventoryBucket.IsSet() && !pItem->GetItemInventoryBucketUseRulesetId().IsEmpty())
	{
		if (UWorld* const World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (auto pGISubsystem = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
				{
					if (URH_CatalogSubsystem* CatalogSubsystem = pGISubsystem->GetCatalogSubsystem())
					{
						if (!CatalogSubsystem->CanRulesetUsePlatformForBucket(pItem->GetItemInventoryBucketUseRulesetId(), RH_GetInventoryBucketFromPlatform(PlayerInfo->GetLoggedInPlatform()), eInventoryBucket.GetValue()))
						{
							return;
						}
					}
				}
			}
		}
	}

	// If the new item cant be added to the pending order, kick off a new one
	if (PendingOrder && !CanAddItemToPendingOrder(StoreItem, PurchaseRefId))
	{
		QueuedOrders.Add(PendingOrder);
		PendingOrder = nullptr;
	}

	if (!PendingOrder)
	{
		PendingOrder = NewObject<URHOrder>(this);
	}

	// Add our new item to the order we are building
	if (PendingOrder)
	{
		// Reset our display timer each time a new order is appended to us
		PendingOrder->OrderType = OrderType;

		if (UOrderItemData* OrderData = NewObject<UOrderItemData>(this))
		{
			OrderData->StoreItem = StoreItem;
			OrderData->Quantity = Quantity;

			PendingOrder->OrderItems.Add(OrderData);
		}

		PendingOrder->TimeSinceCreation = 0.f;
		PendingOrder->PurchaseRefId = PurchaseRefId;

		// Don't group voucher orders together
		if (PendingOrder->OrderType == ERHOrderType::Voucher)
		{
			QueuedOrders.Add(PendingOrder);
			PendingOrder = nullptr;
			DisplayNextOrder();
		}
	}
}

bool URHOrderSubsystem::CanViewRedirectToOrder(TArray<ERHOrderType> ValidTypes)
{
	if (!CanDisplayOrderSharedCheck())
	{
		return false;
	}

	if (QueuedOrders.Num())
	{
		return ValidTypes.Contains(QueuedOrders[0]->OrderType);
	}

	return false;
}

bool URHOrderSubsystem::CanDisplayOrder()
{
	if (!OnOrderReady.IsBound())
	{
		return false;
	}

	if (!CanDisplayOrderSharedCheck())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

    if (APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(World))
    {
        if (ARHHUDCommon* HUD = Cast<ARHHUDCommon>(LocalPC->GetHUD()))
        {
            //Don't allow the next order in the queue to fire if we aren't in the lobby
            if (!HUD->IsLobbyHUD())
            {
                return false;
            }

            if (URHViewManager* ViewManager = HUD->GetViewManager())
            {
                return ViewManager->HasCompletedRedirectFlow(EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AccountLogin)
					&& !ViewManager->IsBlockingOrders()
					&& ViewManager->GetViewRouteCount() > 0;
            }
        }
    }

    return false;
}

bool URHOrderSubsystem::CanDisplayOrderSharedCheck()
{
	return true;
}

bool URHOrderSubsystem::CanAddItemToPendingOrder(URHStoreItem* StoreItem, const FGuid& PurchaseRefId) const
{
    //If there is no pending order return false
    if (!PendingOrder)
    {
        return false;
    }

    //Check for matching PurchaseRefId (but allow battlepass/loot box orders with different purchase IDs to combine)
    if (PendingOrder->PurchaseRefId != PurchaseRefId && PendingOrder->OrderType != ERHOrderType::BattlePass && PendingOrder->OrderType != ERHOrderType::LootBox)
    {
        return false;
    }

    //Check for matching OrderType
    if (PendingOrder->OrderType != GetItemOrderType(StoreItem))
    {
        return false;
    }

    return true;
}

ERHOrderType URHOrderSubsystem::GetItemOrderType(URHStoreItem* StoreItem) const
{
    if (!StoreItem)
    {
        return ERHOrderType::Generic;
    }

	// Check if the item is from a loot box
	if (URHLootBoxSubsystem* LootBoxSubsystem = GetLootBoxManager(GetOuter()))
	{
		if (LootBoxSubsystem->IsFromLootBox(StoreItem))
		{
			return ERHOrderType::LootBox;
		}
	}

    //Check for a currency item being a voucher or active boost
    if (TSoftObjectPtr<UPlatformInventoryItem> InventoryItem = StoreItem->GetInventoryItem())
    {
        if (InventoryItem.IsValid())
        {
            if (URHCurrency* CurrencyItem = Cast<URHCurrency>(InventoryItem.Get()))
            {
                if (CurrencyItem->IsDLCVoucher)
                {
                    return ERHOrderType::Voucher;
                }

            }
        }
    }

	return ERHOrderType::Generic;
}