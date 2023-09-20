// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "Engine/AssetManager.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "RH_ConfigSubsystem.h"
#include "RH_Properties.h"
#include "PInv_AssetManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPInv_AssetManager, Log, All);

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API UPInv_AssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UPInv_AssetManager();
    virtual void PostInitProperties() override;
	FORCEINLINE bool HasCompletedInitialAssetScan() const { return bHasCompletedInitialAssetScan; }
	virtual void StartInitialLoading() override;
	virtual void PostInitialAssetScan() override;

	bool GetPrimaryAssetDataByItemId(const FRH_ItemId& ItemId, FAssetData& OutAssetData) const;
	bool GetPrimaryAssetDataByLootId(const FRH_LootId& LootId, FAssetData& OutAssetData) const;

	//NOTE Only works if the Asset is either loaded or a native class
	template<class T>
	TSoftObjectPtr<T> GetSoftPrimaryAssetByItemId(const FRH_ItemId& ItemId) const;
	//NOTE Only works if the Asset is either loaded or a native class
	template<class T>
	TSoftObjectPtr<T> GetSoftPrimaryAssetByLootId(const FRH_LootId& LootId) const;

	virtual bool ShouldScanPrimaryAssetTypeForItemId(const FPrimaryAssetTypeInfo& TypeInfo) const;
	virtual bool ShouldScanPrimaryAssetTypeForLootId(const FPrimaryAssetTypeInfo& TypeInfo) const;
    virtual bool ShouldScanPrimaryAssetTypeForCollectionContainer(const FPrimaryAssetTypeInfo& TypeInfo) const;

	const TMap<FRH_ItemId, FPrimaryAssetId>& GetItemIdMap() const { return ItemIdToPrimaryAssetIdMap; }

    bool GetPrimaryAssetIdListByCollectionQuery(const FGameplayTagQuery& InCollectionQuery, TArray<FPrimaryAssetId>& OutPrimaryAssetIds) const;
    const TMap<FPrimaryAssetId, FGameplayTagContainer>& GetItemCollectionMap() const { return ItemCollectionMap; }

    template <class T>
    TArray<TSoftObjectPtr<T>> GetAllItemsForTag(FGameplayTag GameplayTag, bool bIncludeChildrenTags = false) const;

	// Reads the settings configuration in for disabled items
	void InitializeDisabledItems(URH_ConfigSubsystem* ConfigSubsystem);

	// Returns if the item is currently fully disabled
	bool IsItemIdDisabled(const FRH_ItemId& ItemId) const { return DisabledItemIds.Contains(ItemId); }

	// Returns if the item is currently temp disabled
	bool IsItemIdTempDisabled(const FRH_ItemId& ItemId) const { return TempDisabledItemIds.Contains(ItemId); }

#if WITH_EDITOR
    /** Resets all asset manager data, called in the editor to reinitialize the config */
    virtual void ReinitializeFromConfig() override;

    /** Changes the default management rules for a specified type */
    virtual void SetPrimaryAssetTypeRules(FPrimaryAssetType PrimaryAssetType, const FPrimaryAssetRules& Rules) override;

    /** Changes the management rules for a specific asset, this overrides the type rules. If passed in Rules is default, delete override */
    virtual void SetPrimaryAssetRules(FPrimaryAssetId PrimaryAssetId, const FPrimaryAssetRules& Rules) override;

	virtual void ApplyPrimaryAssetLabels() override;

	virtual void RefreshPrimaryAssetDirectory(bool bForceRefresh = false);

    void InternalApplyPrimaryAssetLabels(const FPrimaryAssetType& InType);

    /** Gets package names to add to the cook, and packages to never cook even if in startup set memory or referenced */
    virtual void ModifyCook(TConstArrayView<const ITargetPlatform*> TargetPlatforms, TArray<FName>& PackagesToCook, TArray<FName>& PackagesToNeverCook);
#endif

protected:
	virtual bool TryUpdateCachedAssetData(const FPrimaryAssetId& PrimaryAssetId, const FAssetData& NewAssetData, bool bAllowDuplicates) override;

#if WITH_EDITOR
	virtual void RemovePrimaryAssetId(const FPrimaryAssetId& PrimaryAssetId) override;
#endif

	virtual void InternalPostInitialAssetScan();
	virtual void AddOrUpdateItemIdMap(const FPrimaryAssetId& PrimaryAssetId, const FAssetData& AssetData, bool bAllowDuplicates);
	virtual void AddOrUpdateLootIdMap(const FPrimaryAssetId& PrimaryAssetId, const FAssetData& AssetData, bool bAllowDuplicates);
    virtual void AddOrUpdateCollectionContainerMap(const FPrimaryAssetId& PrimaryAssetId, const FAssetData& AssetData, bool bAllowDuplicates);

#if WITH_EDITOR
	virtual void RemoveFromItemIdMap(const FPrimaryAssetId& PrimaryAssetId);
	virtual void RemoveFromLootIdMap(const FPrimaryAssetId& PrimaryAssetId);
    virtual void RemoveFromCollectionContainerMap(const FPrimaryAssetId& PrimaryAssetId);

	virtual void RemovePrimaryAssetFromMap(const FPrimaryAssetId& PrimaryAssetId, TMap<FRH_LootId, FPrimaryAssetId>& IdToAssetMap, TMap<FPrimaryAssetId, FRH_LootId>& AssetToIdMap);
    virtual void RemovePrimaryAssetFromMap(const FPrimaryAssetId& PrimaryAssetId, TMap<FRH_ItemId, FPrimaryAssetId>& IdToAssetMap, TMap<FPrimaryAssetId, FRH_ItemId>& AssetToIdMap);
#endif

protected:

	// These are lists of assets that are currently disabled or temporarily disabled by configuration on the core by their item ids.
	TSet<FRH_ItemId> DisabledItemIds;
	TSet<FRH_ItemId> TempDisabledItemIds;

private:
	UPROPERTY()
	bool bHasCompletedInitialAssetScan;

    // TODO: Consider creating a tuple of int32s for Loot/Item Ide and reduce this to a single set of maps
	// A map of Hi-Rez database Item Ids to Primary Asset Ids
	TMap<FRH_ItemId, FPrimaryAssetId> ItemIdToPrimaryAssetIdMap;
	// A map of Primary Asset Ids to Hi-Rez database Item Ids
	TMap<FPrimaryAssetId, FRH_ItemId> PrimaryAssetIdToItemIdMap;
	// A map of Hi-Rez database Loot Ids to Primary Asset Ids
	TMap<FRH_LootId, FPrimaryAssetId> LootIdToPrimaryAssetIdMap;
	// A map of Primary Asset Ids to Hi-Rez database Item Ids
	TMap<FPrimaryAssetId, FRH_LootId> PrimaryAssetIdToLootIdMap;

    TMap<FPrimaryAssetId, FGameplayTagContainer> ItemCollectionMap;
    TMultiMap<FGameplayTag, FPrimaryAssetId> ItemsByGameplayTagMap;

public:
	static const FPrimaryAssetType PlatformInventoryItemType;
	static const FPrimaryAssetType PlatformStoreAssetType;
    static const FPrimaryAssetType ItemCollectionType;

    /** Return PInv settings object */
    const class UPInv_AssetManagerSettings& GetPInvSettings() const;

private:
    mutable const class UPInv_AssetManagerSettings* CachedPInvSettings;

protected:
    UPROPERTY()
    bool bIsQuickCook;

    UPROPERTY()
    TArray<FString> AdditionalQuickCookPrimaryAssets;

public:
    FName GetCookProfile() const { return CookProfile; }

protected:
    UPROPERTY()
    FName CookProfile;
};

template<class T>
TSoftObjectPtr<T> UPInv_AssetManager::GetSoftPrimaryAssetByItemId(const FRH_ItemId& ItemId) const
{
	FAssetData ItemAssetData;
	if (GetPrimaryAssetDataByItemId(ItemId, ItemAssetData))
	{
		UClass* ItemClass = ItemAssetData.GetClass();
		if (ItemClass != nullptr && ItemClass->IsChildOf(T::StaticClass()))
		{
			return TSoftObjectPtr<T>(ItemAssetData.ToSoftObjectPath());
		}
	}

	return TSoftObjectPtr<T>();
}

template<class T>
TSoftObjectPtr<T> UPInv_AssetManager::GetSoftPrimaryAssetByLootId(const FRH_LootId& LootId) const
{
	FAssetData ItemAssetData;
	if (GetPrimaryAssetDataByLootId(LootId, ItemAssetData))
	{
		UClass* ItemClass = ItemAssetData.GetClass();
		if (ItemClass != nullptr && ItemClass->IsChildOf(T::StaticClass()))
		{
			return TSoftObjectPtr<T>(ItemAssetData.ToSoftObjectPath());
		}
	}

	return TSoftObjectPtr<T>();
}

template <class T>
TArray<TSoftObjectPtr<T>> UPInv_AssetManager::GetAllItemsForTag(FGameplayTag GameplayTag, bool bIncludeChildrenTags /*= false*/) const
{
    TArray<TSoftObjectPtr<T>> Items;
    TArray<FGameplayTag> TagsToFind;

    if (bIncludeChildrenTags)
    {
        FGameplayTagContainer AllGameplayTags = UGameplayTagsManager::Get().RequestGameplayTagChildren(GameplayTag);

        AllGameplayTags.GetGameplayTagArray(TagsToFind);
    }

    TagsToFind.Add(GameplayTag);

    for (const FGameplayTag& Tag : TagsToFind)
    {
        TArray<FPrimaryAssetId> AssetIds;
        ItemsByGameplayTagMap.MultiFind(Tag, AssetIds);

        for (const FPrimaryAssetId& AssetId : AssetIds)
        {
            FAssetData AssetData;
            if (GetPrimaryAssetData(AssetId, AssetData))
            {
                if (AssetData.GetClass() != nullptr && AssetData.GetClass()->IsChildOf<T>())
                {
                    Items.Add(TSoftObjectPtr<T>(AssetData.ToSoftObjectPath()));
                }
            }
        }
    }

    return Items;
}