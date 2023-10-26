// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Lobby/Widgets/RHPurchaseModal.h"

// sets up delegate binding
void URHPurchaseModal::SetupBindings()
{
    // bind to match status update
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
		{
			StoreSubsystem->OnPurchaseItem.AddDynamic(this, &URHPurchaseModal::HandleShowPurchaseModal);
		}
		else
		{
			UE_LOG(RallyHereStart, Warning, TEXT("URHPurchaseModal::SetupBindings failed to get StoreSubsystem-- delegates will not be bound."));
		}
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHPurchaseModal::SetupBindings Warning: GameInstance is not currently valid."));
	}
}

// handles a store data factory request to show the modal
void URHPurchaseModal::HandleShowPurchaseModal_Implementation(URHStoreItem* Item, URHStoreItemPrice* Price)
{
    // show yourself! (override this in BP)
    ShowWidget();
}
