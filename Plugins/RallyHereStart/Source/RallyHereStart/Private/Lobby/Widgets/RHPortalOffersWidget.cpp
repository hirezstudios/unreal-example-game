// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Lobby/Widgets/RHPortalOffersWidget.h"

TArray<URHStoreItem*> URHPortalOffersWidget::GetPortalOfferItems() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
		{
			return StoreSubsystem->GetStoreItemsForVendor(StoreSubsystem->GetPortalOffersVendorId(), false, false);
		}
	}

    return TArray<URHStoreItem*>();
}
