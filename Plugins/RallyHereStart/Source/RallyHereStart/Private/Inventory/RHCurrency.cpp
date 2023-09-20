// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "Inventory/RHCurrency.h"

URHCurrency::URHCurrency(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectInitializer)
{
	static const FGameplayTag ItemCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::CurrencyCollectionName);
	CollectionContainer.AddTag(ItemCollectionTag);
	
	IsOwnableInventoryItem = true;

	FullSplashIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("FullSplashIconInfo"));
	SmallIconIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("SmallIconIconInfo"));

	CanOwnMultiple = true;
}

bool URHCurrency::GetCurrencyDataForQuantity(int32 Quantity, FCurrencyImageRow& Data) const
{
	if (CurrencyDataByQtyTable.IsNull())
	{
		return false;
	}

	if (!CurrencyDataByQtyTable.IsValid())
	{
		CurrencyDataByQtyTable.LoadSynchronous();
	}

	UDataTable* pCurrencyDataByQtyTable = CurrencyDataByQtyTable.Get();
    if (pCurrencyDataByQtyTable != nullptr)
    {
		static const FString strGetQty(TEXT("GetCurrencyDataForQuantity"));

        TArray<FCurrencyImageRow*> Rows;
		pCurrencyDataByQtyTable->GetAllRows(strGetQty, Rows);

        for (int32 i = 1; i < Rows.Num(); ++i)
        {
            if (Rows[i]->Quantity > Quantity)
            {
                Data = *Rows[i - 1];
                return true;
            }
        }

        // Return the last row if we never found a limit on our quantity 
        if (Rows.Num())
        {
            Data = *Rows[Rows.Num() - 1];
            return true;
        }
    }

    return false;
}

bool URHCurrency::GetProductSkuForQuantity(int32 Quantity, FString& SKU)
{
	FCurrencyImageRow QuantityData;

	if (GetCurrencyDataForQuantity(Quantity, QuantityData))
	{
		EExternalSkuSource SkuSource = GetSkuSource();

		if (SkuSource != EExternalSkuSource::ESS_No_Souce)
		{
			if (auto pSku = QuantityData.ExternalProductSkus.Find(SkuSource))
			{
				SKU = *pSku;
				return true;
			}
		}
	}

	return GetProductSku(SKU);
}

#if WITH_EDITOR
void URHCurrency::GetPreloadDependencies(TArray<UObject*>& OutDeps)
{
	Super::GetPreloadDependencies(OutDeps);

	if (FullSplashIconInfo != nullptr)
	{
		OutDeps.Add(FullSplashIconInfo);
	}

	if (SmallIconIconInfo != nullptr)
	{
		OutDeps.Add(SmallIconIconInfo);
	}
}
#endif

