// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/Widgets/RHCurrencyDisplay.h"

void URHCurrencyDisplay::SetInventoryCountOnText(const FRH_ItemId& ItemId, UTextBlock* TextBlock)
{
	if (MyHud != nullptr)
	{
		if (URH_PlayerInfo* PlayerInfo = MyHud->GetLocalPlayerInfo())
		{
			if (URH_PlayerInventory* PlayerInventory = PlayerInfo->GetPlayerInventory())
			{
				PlayerInventory->GetInventoryCount(ItemId, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [TextBlock](int32 Count)
					{
						TextBlock->SetText(FText::AsNumber(Count));
					}));
			}
		}
	}
}