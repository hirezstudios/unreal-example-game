// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "GameFramework/RHGameInstance.h"
#include "Subsystems/RHLocalDataSubsystem.h"
#include "Lobby/Widgets/RHLoginInventoryCheck.h"
#include "Subsystems/RHStoreSubsystem.h"

bool URHLoginInventoryCheckViewRedirector::ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData)  //$$ KAB - Route names changed to Gameplay Tags
{
    // Always return true at this point in the login, having this redirector have duplicate logic would lead to error prone updates
    return true;
}

void URHLoginInventoryCheck::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();

	bHasRequiredVendors = false;
}

//$$ LDP - Added Visibility param
void URHLoginInventoryCheck::ShowWidget(ESlateVisibility InVisibility /* = ESlateVisibility::SelfHitTestInvisible */)
{
    Super::ShowWidget(InVisibility);

	UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::ShowWidget()"));

	if (URHLocalDataSubsystem* LocalDataSubsystem = MyHud->GetLocalDataSubsystem())
	{
		LocalDataSubsystem->OnPlayerInventoryReady.AddUObject(this, &URHLoginInventoryCheck::OnPlayerInventoryReceived);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
		{
			TArray<int32> VendorIds;

			// Add Any required vendors for login screen to complete here:

			if (VendorIds.Num())
			{
				StoreSubsystem->RequestVendorData(VendorIds, FRH_CatalogCallDelegate::CreateLambda([this](bool bSuccess)
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
		RemoveViewRoute(MyRouteTag); //$$ KAB - Route names changed to Gameplay Tags
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

	if (URHLocalDataSubsystem* LocalDataSubsystem = MyHud->GetLocalDataSubsystem())
	{
		if (!LocalDataSubsystem->HasFullInventory())
		{
			UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() STILL WAITING: Player Inventory Missing"));
			return false;
		}

		UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() SUCCESS"));
		return true;
	}

	UE_LOG(RallyHereStart, Log, TEXT("URHLoginInventoryCheck::HasRequiredInventory() ERROR: Game Instance or UI Session Manager Missing"));
    return false;
}
