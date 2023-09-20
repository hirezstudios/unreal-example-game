// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Inventory/IconInfo.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHCurrency.generated.h"

USTRUCT(BlueprintType)
struct FCurrencyImageRow : public FTableRowBase
{
    GENERATED_BODY()

    // The amount of currency threshold
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Quantity;

	// The quantity of bonus currency included at this amount
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BonusQuantity;
	
	// The Image used for that quantity of currency
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Image;

	// The Full Splash Image used for that quantity of currency
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> FullSplashImage;

	// Map of product skus to quantity
	UPROPERTY(EditDefaultsOnly, Category = "Skus")
	TMap<EExternalSkuSource, FString> ExternalProductSkus;

	FCurrencyImageRow() :
		Quantity(0),
		BonusQuantity(0)
	{
	}
};

UCLASS()
class RALLYHERESTART_API URHCurrency : public UPlatformInventoryItem
{
    GENERATED_BODY()

public:
    URHCurrency(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // Flags the item as a Voucher, used for redeeming DLC
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool IsDLCVoucher;

    // Order that currency is displayed if there are multiple currencies on a price
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 SortOrder;

    // Table that contains progressive item information based on quantity
    UPROPERTY(EditDefaultsOnly, meta = (AssetBundles = "Data"))
    TSoftObjectPtr<UDataTable> CurrencyDataByQtyTable;

    // Gets the level of progressive data based on quantity
    UFUNCTION(BlueprintPure)
    bool GetCurrencyDataForQuantity(int32 Quantity, FCurrencyImageRow& Data) const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UIconInfo* GetFullSplash() const { return FullSplashIconInfo; }

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UIconInfo* GetSmallIcon() const { return SmallIconIconInfo; }

	// Returns the product sku for the item based on its quantity
	bool GetProductSkuForQuantity(int32 Quantity, FString& SKU);

#if WITH_EDITOR
	virtual void GetPreloadDependencies(TArray<UObject*>& OutDeps) override;
#endif

protected:
	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Icon")
	UIconInfo* FullSplashIconInfo;

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Icon")
	UIconInfo* SmallIconIconInfo;
};