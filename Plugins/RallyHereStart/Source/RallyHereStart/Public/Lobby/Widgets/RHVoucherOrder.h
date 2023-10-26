// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "RHVoucherOrder.generated.h"

UCLASS()
class RALLYHERESTART_API URHVoucherOrder : public URHWidget
{
    GENERATED_BODY()

public:
    // Displays a popup showing informing the user their voucher failed
    UFUNCTION(BlueprintCallable)
    void DisplayVoucherRedemptionFailed();

    // Returns all the vouchers the player has to redeem, and the items used to purchase them
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Voucher Orders", AutoCreateRefTerm = "Delegate"))
    void GetVoucherOrders(const FRH_GetAffordableItemsInVendorDynamicDelegate& Delegate);

    // Redeem a set of provided vouchers
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Redeem Vouchers", AutoCreateRefTerm = "Delegate"))
    void RedeemVouchers(TArray<URHStoreItem*> VoucherItems, const FRH_CatalogCallDynamicDelegate& Delegate);

private:
	URH_RedeemVouchersAsyncTaskHelper* RedeemVouchersHelper;
};

DECLARE_DELEGATE(OnRedeemVouchersAsyncTaskComplete)

UCLASS()
class URH_RedeemVouchersAsyncTaskHelper : public UObject
{
	GENERATED_BODY()

public:
	void Start();
	bool IsCompleted() const { return RequestsCompleted >= VoucherItems.Num(); }

	int32 RequestsCompleted;
	OnRedeemVouchersAsyncTaskComplete OnCompleteDelegate;

	UPROPERTY()
	const URH_PlayerInfo* PlayerInfo;

	UPROPERTY(Transient)
	TArray<URHStoreItem*> VoucherItems;

	UPROPERTY(Transient)
	TArray<URHStorePurchaseRequest*> PurchaseRequests;

private:
	void CanAffordCheck(bool bOwned, URHStoreItem* VoucherItem, URHStoreItemPrice* Price);
};
