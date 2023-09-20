// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/Widgets/RHPortalOffersWidget.h"

URHStoreItemHelper* URHPortalOffersWidget::GetItemHelper() const
{
    if (MyHud.IsValid())
    {
        return MyHud->GetItemHelper();
    }
    return nullptr;
}

TArray<URHStoreItem*> URHPortalOffersWidget::GetPortalOfferItems() const
{
    if (URHStoreItemHelper* StoreItemHelper = GetItemHelper())
    {
        return StoreItemHelper->GetStoreItemsForVendor(StoreItemHelper->GetPortalOffersVendorId(), false, false);
    }

    return TArray<URHStoreItem*>();
}
