// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "GameFramework/RHGameInstance.h"
#include "Managers/RHOrderManager.h"
#include "Managers/RHViewManager.h"
#include "Lobby/Widgets/RHOrderModal.h"

bool URHOrderViewRedirector::ShouldRedirect(ARHHUDCommon* HUD, FName Route, UObject*& SceneData)
{
	if (UWorld* World = GetWorld())
	{
		if (URHGameInstance* GameInstance = World->GetGameInstance<URHGameInstance>())
		{
			if (URHOrderManager* OrderManager = GameInstance->GetOrderManager())
			{
				if (OrderManager->CanViewRedirectToOrder(ValidOrderTypes))
				{
					if (URHViewManager* ViewManager = HUD->GetViewManager())
					{
						FViewRoute ViewRoute;
						if (ViewManager->GetViewRoute(Route, ViewRoute))
						{
							if (!ViewRoute.BlockOrders)
							{
								SceneData = OrderManager->GetNextOrder();
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

void URHOrderModal::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	// Json Overrides - If we forget to update the data, this allows us to on the fly add in a header whenever needed.
	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		if (URHJsonDataFactory* JsonDataFactory = Cast<URHJsonDataFactory>(GameInstance->GetJsonDataFactory()))
		{
			TSharedPtr<FJsonObject> OrderHeaderOverrides = JsonDataFactory->GetJsonPanelByName(TEXT("store"));

			if (OrderHeaderOverrides.IsValid())
			{
				const TArray<TSharedPtr<FJsonValue>>* Overrides;

				if (OrderHeaderOverrides.Get()->TryGetArrayField(TEXT("orderHeaders"), Overrides))
				{
					for (TSharedPtr<FJsonValue> Override : *Overrides)
					{
						TSharedPtr<FJsonObject> OverrideObj = Override->AsObject();

						if (OverrideObj.IsValid())
						{
							const TSharedPtr<FJsonObject>* ObjectField;

							if (OverrideObj.Get()->TryGetObjectField(TEXT("header"), ObjectField))
							{
								const TArray<TSharedPtr<FJsonValue>>* OverrideLootIds;
								FText HeaderOverride = FText::FromString(URHJsonDataFactory::GetLocalizedStringFromObject(ObjectField));

								if (OverrideObj.Get()->TryGetArrayField(TEXT("overrideLootIds"), OverrideLootIds))
								{
									for (const TSharedPtr<FJsonValue>& JsonValue : *OverrideLootIds)
									{
										int32 lootId;
										if (JsonValue->TryGetNumber(lootId))
										{
											HeaderOverridesFromJson.Add(lootId, HeaderOverride);
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

URHOrderManager* URHOrderModal::GetOrderManager() const
{
    if (MyHud.IsValid())
    {
        return MyHud->GetOrderManager();
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHOrderModal::OrderManager Warning: MyHud is not currently valid."));
    }
    return nullptr;
}

FText URHOrderModal::GetHeaderText(const URHOrder* Order) const
{
	if (Order)
	{
		if (HeaderOverridesTable)
		{
			static const FString strGetRows(TEXT("GetHeaderOverrideData"));

			TArray<FOrderHeaderOverrides*> Rows;
			HeaderOverridesTable->GetAllRows(strGetRows, Rows);

			for (int32 i = 0; i < Rows.Num(); ++i)
			{
				for (int32 j = 0; j < Order->OrderItems.Num(); ++j)
				{
					if (Order->OrderItems[j] && Rows[i]->LootTableItemIds.Contains(Order->OrderItems[j]->GetLootId()))
					{
						return Rows[i]->Header;
					}
				}
			}
		}

		// Check if we have any JSON overrides, only process if in use
		if (HeaderOverridesFromJson.Num())
		{
			for (int32 i = 0; i < Order->OrderItems.Num(); ++i)
			{
				if (const FText* Header = HeaderOverridesFromJson.Find(Order->OrderItems[i]->GetLootId()))
				{
					return *Header;
				}
			}
		}

		if (Order->IsPurchase())
		{
			return NSLOCTEXT("RHOrder", "PurchaseComplete", "PURCHASE COMPLETE");
		}

		return NSLOCTEXT("RHOrder", "ItemAcquired", "ITEM ACQUIRED");
	}

	return FText();
}
