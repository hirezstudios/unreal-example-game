// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

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

	//$$ KAB - REMOVED - Move Loot Id from StoreAsset to InventoryItem

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