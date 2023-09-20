// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RH_Properties.h"
#include "PlatformStoreAsset.generated.h"

DECLARE_DELEGATE(FRH_OnOwnershipCompleteDelegate);

class URHStoreItem;

UCLASS()
class URH_PlatformStoreAssetOwnershipHelper : public UObject
{
	GENERATED_BODY()

public:
	void StartCanOwnMoreCheck();
	void StartIsOwnedCheck();

	FRH_OnOwnershipCompleteDelegate OnCompleteDelegate;
	bool IsComplete;
	FRH_GetInventoryStateBlock Delegate;
	int32 Results;
	bool HasResponded;
	TArray<URHStoreItem*> ItemsToCheck;

	UPROPERTY()
	const URH_PlayerInfo* PlayerInfo;

private:
	void OnOwnMoreResponse(bool bSuccess);
	void OnIsOwnedResponse(bool bSuccess);

};

UCLASS()
class RALLYHERESTART_API UPlatformStoreAsset : public UPlatformInventoryItem
{
    GENERATED_BODY()

public:
    UPlatformStoreAsset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void OverridePerObjectConfigSection(FString& SectionName) override;

    static const FName StoreAssetCollectionName;

    // Returns the item id associated with the item that matches the value in the database
    UFUNCTION(BlueprintPure, Category = "Item")
    const FRH_LootId& GetLootId() const { return LootId; }

    // The loot id associated with the item, this is used on Store Assets because Bundles and other assets that these represent don't have physical item IDs, they only exist as a loot id
    UPROPERTY(EditDefaultsOnly, AssetRegistrySearchable, DuplicateTransient, Category = "Item")
    FRH_LootId LootId;

	virtual void IsOwned(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate) override;
	virtual void CanOwnMore(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate) override;

public:
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;

	FDelegateHandle ReadyForBundleDataHandle;
#endif

protected:
	UPROPERTY(Transient)
	TArray<URH_PlatformStoreAssetOwnershipHelper*> PendingOwnershipRequests;
};