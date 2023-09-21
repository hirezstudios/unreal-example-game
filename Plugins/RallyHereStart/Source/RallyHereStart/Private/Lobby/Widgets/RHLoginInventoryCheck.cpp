// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "GameFramework/RHGameInstance.h"
#include "Lobby/Widgets/RHLoginInventoryCheck.h"
#include "Managers/RHStoreItemHelper.h"

bool URHLoginInventoryCheckViewRedirector::ShouldRedirect(ARHHUDCommon* HUD, FName Route, UObject*& SceneData)
{
    // Always return true at this point in the login, having this redirector have duplicate logic would lead to error prone updates
    return true;
}

void URHLoginInventoryCheck::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();

	bHasRequiredVendors = false;
}

void URHLoginInventoryCheck::ShowWidget()
{
    Super::ShowWidget();

	UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::ShowWidget()"));

	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		if (URHUISessionManager* SessionManager = GameInstance->GetUISessionManager())
		{
			if (URHUISessionData* SessionData = SessionManager->SessionDataPerPlayer.FindRef(MyHud->GetLocalPlayerInfo()))
			{
				SessionData->OnPlayerInventoryReady.AddUObject(this, &URHLoginInventoryCheck::OnPlayerInventoryReceived);
			}
		}
	}

	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		if (URHStoreItemHelper* StoreItemHelper = GameInstance->GetStoreItemHelper())
		{
			TArray<int32> VendorIds;

			// Add Any required vendors for login screen to complete here:

			if (VendorIds.Num())
			{
				StoreItemHelper->RequestVendorData(VendorIds, FRH_CatalogCallDelegate::CreateLambda([this](bool bSuccess)
					{
						bHasRequiredVendors = true;
						UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::OnStoreVendorsLoaded()"));
						CheckRequiredInventory();
					}));
			}
			else
			{
				bHasRequiredVendors = true;
			}
		}
	}

	// Do our initial check next frame so that our screen doesn't get closed while opening if all checks pass
	FTimerDelegate TimerCallback;
	TimerCallback.BindWeakLambda(this, [this]()
		{
			CheckRequiredInventory();
		});

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, 0.1f, false);
}

void URHLoginInventoryCheck::CancelLogin()
{
    if (ARHLobbyHUD* LobbyHUD = Cast<ARHLobbyHUD>(MyHud))
    {
        if (URHLoginDataFactory* LoginDataFactory = LobbyHUD->GetLoginDataFactory())
        {
            LoginDataFactory->LogOff(false);
        }
    }
}

void URHLoginInventoryCheck::OnPlayerInventoryReceived()
{
	UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::OnPlayerInventoryReceived()"));
    CheckRequiredInventory();
}

bool URHLoginInventoryCheck::CheckRequiredInventory()
{
    // If we have everything we need, proceed
    if (HasRequiredInventory())
    {
		RemoveViewRoute(MyRouteName);
		return true;
    }

    return false;
}

bool URHLoginInventoryCheck::HasRequiredInventory()
{
	UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() BEGIN"));

	if (!bHasRequiredVendors)
	{
		UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() STILL WAITING: Required Vendors Not Loaded"));
		return false;
	}

	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		if (URHUISessionManager* SessionManager = GameInstance->GetUISessionManager())
		{
			if (!SessionManager->HasFullInventory(MyHud->GetLocalPlayerInfo()))
			{
				UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() STILL WAITING: Player Inventory Missing"));
				return false;
			}

			UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() SUCCESS"));
			return true;
		}
    }

	UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() ERROR: Game Instance or UI Session Manager Missing"));
    return false;
}
