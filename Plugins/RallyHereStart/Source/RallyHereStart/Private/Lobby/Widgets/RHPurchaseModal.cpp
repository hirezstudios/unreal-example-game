// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Lobby/Widgets/RHPurchaseModal.h"

// sets up delegate binding
void URHPurchaseModal::SetupBindings()
{
    // bind to match status update
    if (URHStoreItemHelper* pStoreItemHelper = GetStoreItemHelper())
    {
        pStoreItemHelper->OnPurchaseItem.AddDynamic(this, &URHPurchaseModal::HandleShowPurchaseModal);
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHPurchaseModal::SetupBindings failed to get StoreItemHelper-- delegates will not be bound."));
    }
}

URHStoreItemHelper* URHPurchaseModal::GetStoreItemHelper()
{
    if (MyHud.IsValid())
    {
        return MyHud->GetItemHelper();
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHPurchaseModal::GetStoreItemHelper Warning: MyHud is not currently valid."));
    }
    return nullptr;
}

// handles a store data factory request to show the modal
void URHPurchaseModal::HandleShowPurchaseModal_Implementation(URHStoreItem* Item, URHStoreItemPrice* Price)
{
    // show yourself! (override this in BP)
    ShowWidget();
}
