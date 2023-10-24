// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Managers/RHViewManager.h"
#include "Shared/Widgets/RHWidget.h"

#include "RHLoginInventoryCheck.generated.h"

UCLASS(BlueprintType, Blueprintable)
class RALLYHERESTART_API URHLoginInventoryCheckViewRedirector : public URHViewRedirecter
{
    GENERATED_BODY()

public:
    virtual bool ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData) override; //$$ KAB - Route names changed to Gameplay Tags
};

UCLASS()
class RALLYHERESTART_API URHLoginInventoryCheck : public URHWidget
{
    GENERATED_BODY()

	URHLoginInventoryCheck(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
	}

    virtual void InitializeWidget_Implementation() override;

public:
	// Checks if we are cleared to move on passed this screen
	bool CheckRequiredInventory();
    // Called when player inventory updated
    void OnPlayerInventoryReceived();

    UFUNCTION(BlueprintCallable, Category = "LoginInventoryCheck")
    void CancelLogin();

protected:
    // Checks if the player has all of their required inventory to be allowed to complete login
    bool HasRequiredInventory();

	virtual void ShowWidget(ESlateVisibility InVisibility = ESlateVisibility::SelfHitTestInvisible) override; //$$ LDP - Added visibility param

	bool bHasRequiredVendors;
};