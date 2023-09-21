// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Inventory/RHBattlepass.h"
#include "Inventory/RHCurrency.h"
#include "Managers/RHStoreItemHelper.h"
#include "Managers/RHPopupManager.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/Widgets/RHPurchaseConfirmationWidget.h"

void URHPurchaseConfirmationWidget::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

void URHPurchaseConfirmationWidget::UninitializeWidget_Implementation()
{
    Super::UninitializeWidget_Implementation();
}

URHStoreItemHelper* URHPurchaseConfirmationWidget::GetStoreItemHelper() const
{
    if (MyHud.IsValid())
    {
		return MyHud->GetItemHelper();
    }

    return nullptr;
}

void URHPurchaseConfirmationWidget::PromptAlreadyPurchasing()
{
    if (MyHud.IsValid())
    {
		if (URHPopupManager* PopupManager = MyHud->GetPopupManager())
		{
			FRHPopupConfig AlreadyPurchasingPopup;
			AlreadyPurchasingPopup.Header = NSLOCTEXT("General", "Error", "Error");
			AlreadyPurchasingPopup.Description = NSLOCTEXT("RHPurchaseRequest", "AlreadyPurchasing", "We are still processing your previous purchase. Please try again later.");
			AlreadyPurchasingPopup.CancelAction.AddDynamic(PopupManager, &URHPopupManager::OnPopupCanceled);

			FRHPopupButtonConfig& ConfirmButton = (AlreadyPurchasingPopup.Buttons[AlreadyPurchasingPopup.Buttons.Add(FRHPopupButtonConfig())]);
			ConfirmButton.Label = NSLOCTEXT("General", "Confirm", "Confirm");
			ConfirmButton.Type = ERHPopupButtonType::Confirm;
			ConfirmButton.Action.AddDynamic(PopupManager, &URHPopupManager::OnPopupCanceled);

			PopupManager->AddPopup(AlreadyPurchasingPopup);
		}
    }
}

void URHPurchaseConfirmationWidget::CanChangePurchaseQuantity(int32 QuantityChangeAmount, const FRH_GetInventoryStateBlock& Delegate)
{
    //Return false if we don't have a valid purchase item or the change amount would cause our quantity to be invalid
    if (!PurchaseItem || PurchaseQuantity + QuantityChangeAmount <= 0)
    {
		Delegate.ExecuteIfBound(false);
        return;
    }

	// If we are a battlepass, make sure we can have that many more tiers
	if (URHStoreItemWithBattlepassData* BattlepassPurchaseData = Cast<URHStoreItemWithBattlepassData>(PurchaseRequestData))
	{
		if (URHBattlepass* Battlepass = BattlepassPurchaseData->BattlepassItem)
		{
			if (MyHud != nullptr)
			{
				Battlepass->GetCurrentLevel(MyHud->GetLocalPlayerInfo(), FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [this, Battlepass, QuantityChangeAmount, Delegate](int32 Count)
					{
						int32 NumLevels = Battlepass->GetLevelCount(this);
						if (PurchaseQuantity + QuantityChangeAmount + Count > NumLevels)
						{
							Delegate.ExecuteIfBound(false);
							return;
						}
						Delegate.ExecuteIfBound(true);
					}));
				return;
			}
		}
	}
	
	Delegate.ExecuteIfBound(true);
}

void URHPurchaseConfirmationWidget::TryChangePurchaseQuantity(int32 QuantityChangeAmount, const FRH_GetInventoryStateBlock& Delegate)
{
	CanChangePurchaseQuantity(QuantityChangeAmount, FRH_GetInventoryStateDelegate::CreateWeakLambda(this, [this, QuantityChangeAmount, Delegate](bool bSuccess)
		{
			if (bSuccess)
			{
				PurchaseQuantity += QuantityChangeAmount;
				Delegate.ExecuteIfBound(true);
			}
			else
			{
				Delegate.ExecuteIfBound(false);
			}
		}));
}

void URHPurchaseConfirmationWidget::OnPurchaseComplete(bool bCompletedPurchase)
{
	if (PurchaseRequestData)
	{
		PurchaseRequestData->PurchaseCompletedCallback.Broadcast(bCompletedPurchase);
	}
}