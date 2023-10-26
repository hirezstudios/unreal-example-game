// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Lobby/Widgets/RHStoreWidget.h"
#include "Inventory/RHBattlepass.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHPurchaseConfirmationWidget.generated.h"

class URHWeaponAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPurchaseCompleted, bool, Success);


UCLASS(BlueprintType)
class RALLYHERESTART_API URHPurchaseData : public UObject
{
	GENERATED_BODY()

public:
	// Store Item we want to purchase
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn))
	URHStoreItem* StoreItem;

	// Quantity of the item intended for purchase
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn))
	int32 PurchaseQuantity;

	// Used to specify the location the purchase was made from
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn))
	FString ExternalTransactionId;

	// Called on Purchase Complete if any screen passing in data cares
	UPROPERTY(BlueprintReadWrite)
	FPurchaseCompleted PurchaseCompletedCallback;

	URHPurchaseData() : PurchaseQuantity(1) {}
	URHPurchaseData(URHStoreItem* InStoreItem, int32 InPurchaseQuantity) : StoreItem(InStoreItem), PurchaseQuantity(InPurchaseQuantity) {}
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHStoreSectionItemWithPurchaseData : public URHPurchaseData
{
	GENERATED_BODY()

public:
	// Quantity of the item we want to purchase
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn))
	URHStoreSectionItem* StoreSectionItem;

	URHStoreSectionItemWithPurchaseData() : URHPurchaseData() {}
	URHStoreSectionItemWithPurchaseData(URHStoreItem* InStoreItem, int32 InPurchaseQuantity) : URHPurchaseData(InStoreItem, InPurchaseQuantity) {}
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHStoreItemWithPurchaseData : public URHPurchaseData
{
    GENERATED_BODY()

public:
	URHStoreItemWithPurchaseData() : URHPurchaseData() {}
    URHStoreItemWithPurchaseData(URHStoreItem* InStoreItem, int32 InPurchaseQuantity) : URHPurchaseData(InStoreItem, InPurchaseQuantity) {}
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHStoreItemWithBattlepassData : public URHPurchaseData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn))
	URHBattlepass* BattlepassItem;

	URHStoreItemWithBattlepassData() : URHPurchaseData() {}
	URHStoreItemWithBattlepassData(URHStoreItem* InStoreItem, int32 InPurchaseQuantity) : URHPurchaseData(InStoreItem, InPurchaseQuantity) {}
};

UCLASS()
class RALLYHERESTART_API URHPurchaseConfirmationWidget : public URHWidget
{
    GENERATED_BODY()

public:
    // Initializes the base setup for the widget
    virtual void InitializeWidget_Implementation() override;

    // Cleans up the base setup for the widget
    virtual void UninitializeWidget_Implementation() override;

    UFUNCTION(BlueprintCallable)
    void PromptAlreadyPurchasing();

protected:
    //StoreItem we are currently viewing for purchase
    UPROPERTY(BlueprintReadWrite)
    URHStoreItem* PurchaseItem;

    //Quantity of the StoreItem we are trying to purchase
    UPROPERTY(BlueprintReadWrite)
    int32 PurchaseQuantity;

	// Checks if the purchase quantity can be changed by the given amount
	UFUNCTION(BlueprintCallable, Category = "Purchase Confirmation", meta = (DisplayName = "Can Change Purchase Quantity", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_CanChangePurchaseQuantity(int32 QuantityChangeAmount, const FRH_GetInventoryStateDynamicDelegate& Delegate) { CanChangePurchaseQuantity(QuantityChangeAmount, Delegate); }
	void CanChangePurchaseQuantity(int32 QuantityChangeAmount, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());

	// Changes the quantity if valid, returns if successful
	UFUNCTION(BlueprintCallable, Category = "Purchase Confirmation", meta = (DisplayName = "Try Change Purchase Quantity", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_TryChangePurchaseQuantity(int32 QuantityChangeAmount, const FRH_GetInventoryStateDynamicDelegate& Delegate) { TryChangePurchaseQuantity(QuantityChangeAmount, Delegate); }
	void TryChangePurchaseQuantity(int32 QuantityChangeAmount, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());

	// Called whenever the player completes their purchase or decides not to
	UFUNCTION(BlueprintCallable)
	void OnPurchaseComplete(bool bCompletedPurchase);

	UPROPERTY(BlueprintReadWrite)
	URHPurchaseData* PurchaseRequestData;
};