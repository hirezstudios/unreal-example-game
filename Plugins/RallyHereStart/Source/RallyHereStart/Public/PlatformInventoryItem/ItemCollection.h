// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/AssetManagerTypes.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"

#if WITH_EDITORONLY_DATA
#include "AssetRegistry/ARFilter.h"
#include "PlatformInventoryItem/PInv_AssetManager.h"
#endif

#include "ItemCollection.generated.h"


/**
 *
 */
UCLASS(Abstract)
class RALLYHERESTART_API UItemCollection : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UItemCollection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
    virtual void PostLoad() override;
	virtual void BeginDestroy() override;

    /** Management rules for this specific asset, if set it will override the type rules */
    UPROPERTY(EditAnywhere, Category = Rules, meta = (ShowOnlyInnerProperties))
    FPrimaryAssetRules Rules;

	/** This query is ran against the CollectionContainer on all Items. Anything that matches this query is added to the collection. 
	  * Please not that collections are now populated at runtime rather than cook time to prevent bringing in unnecessary assets.
	  */
	UPROPERTY(EditAnywhere, Category = "Collection")
	FGameplayTagQuery CollectionQuery;

	UPROPERTY(AssetRegistrySearchable)
	FName PrimaryAssetType;

protected:

    virtual void UpdateCollection();
    virtual void TryToAddAsset(const FAssetData& InAsset);

#if WITH_EDITORONLY_DATA
    virtual void UpdateAssetBundleData() override;
#endif

	template<typename T>
	static void AddAsset(TArray< TSoftObjectPtr<T> >& AssetArray, const FAssetData& NewAsset);

private:
	void PostInitialAssetScan();
	FDelegateHandle PostInitialAssetScanHandle;
};

template<typename T>
FORCEINLINE void UItemCollection::AddAsset(TArray< TSoftObjectPtr<T> >& AssetArray, const FAssetData& NewAsset)
{
    if (NewAsset.IsValid() && NewAsset.GetClass()->IsChildOf(T::StaticClass()))
    {
        AssetArray.Add(TSoftObjectPtr<T>(NewAsset.ToSoftObjectPath()));
    }
}
