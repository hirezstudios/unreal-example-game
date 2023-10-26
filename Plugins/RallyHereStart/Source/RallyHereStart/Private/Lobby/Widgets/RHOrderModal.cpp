// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "GameFramework/RHGameInstance.h"
#include "Subsystems/RHNewsSubsystem.h"
#include "Subsystems/RHOrderSubsystem.h"
#include "Managers/RHViewManager.h"
#include "Lobby/Widgets/RHOrderModal.h"

bool URHOrderViewRedirector::ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData) //$$ KAB - Route names changed to Gameplay Tags
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance<UGameInstance>())
		{
			if (URHOrderSubsystem* OrderSubsystem = GameInstance->GetSubsystem<URHOrderSubsystem>())
			{
				if (OrderSubsystem->CanViewRedirectToOrder(ValidOrderTypes))
				{
					if (URHViewManager* ViewManager = HUD->GetViewManager())
					{
						FViewRoute ViewRoute;
						if (ViewManager->GetViewRoute(RouteTag, ViewRoute)) //$$ KAB - Route names changed to Gameplay Tags
						{
							if (!ViewRoute.BlockOrders)
							{
								SceneData = OrderSubsystem->GetNextOrder();
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
		if (URHNewsSubsystem* pNewsSubsystem = Cast<URHNewsSubsystem>(GameInstance->GetSubsystem<URHNewsSubsystem>()))
		{
			TSharedPtr<FJsonObject> OrderHeaderOverrides = pNewsSubsystem->GetJsonPanelByName(TEXT("store"));

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
								FText HeaderOverride = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField));

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

URHOrderSubsystem* URHOrderModal::GetOrderSubsystem() const
{
    if (MyHud.IsValid())
    {
        return MyHud->GetOrderSubsystem();
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHOrderModal::GetOrderSubsystem Warning: MyHud is not currently valid."));
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
