// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/IconInfo.h"
#include "Inventory/RHEvent.h"
#include "RH_PlayerInventory.h"
#include "Managers/RHStoreItemHelper.h"
#include "RHBattlepass.generated.h"

UENUM(BlueprintType)
enum class EBattlepassTrackType : uint8
{
	EFreeTrack        UMETA(DisplayName = "Free Items"),
	EPremiumTrack     UMETA(DisplayName = "Premium Items"),
	EInstantUnlock    UMETA(DisplayName = "Purchase Unlocks"),
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHBattlepassRewardItem : public UObject
{
	GENERATED_BODY()

public:
	// The specific reward item
	UPROPERTY(BlueprintReadOnly, Category = "Battlepass Reward Item")
	URHStoreItem* Item;

	// Which track this item is on
	UPROPERTY(BlueprintReadOnly, Category = "Battlepass Reward Item")
	EBattlepassTrackType Track;

	// Gets the level of the reward
	UFUNCTION(BlueprintPure, Category = "Battlepass Reward Item")
	int32 GetRewardLevel() const { return Item ? Item->GetSortOrder() : 0; }
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHBattlepassLevel : public UObject
{
	GENERATED_BODY()

public:

	// The Xp span of the level 
	UFUNCTION(BlueprintPure, Category = "Battlepass Level")
	int32 GetXPSpan() const { return EndXp - StartXp; }

	// The Number of the level
	UPROPERTY(BlueprintReadOnly, Category = "Battlepass Level")
	int32 LevelNumber;

	// The Total Xp required to be at the given level
	UPROPERTY(BlueprintReadOnly, Category = "Battlepass Level")
	int32 StartXp;

	// The Total Xp required to be at the end of the level
	UPROPERTY(BlueprintReadOnly, Category = "Battlepass Level")
	int32 EndXp;

	// List of reward items for the level
	UPROPERTY(BlueprintReadOnly, Category = "Battlepass Level")
	TArray<URHBattlepassRewardItem*> RewardItems;
};

UCLASS()
class RALLYHERESTART_API URHBattlepass : public URHEvent
{
	GENERATED_BODY()
	
public:
    URHBattlepass(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// The battlepass XP item Id
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FRH_ItemId ProgressItemId;

	// The vendor id of the instant unlock rewards
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InstantUnlockRewardVendorId;

	// The vendor id of the free rewards
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 FreeRewardVendorId;

	// The vendor id of the premium rewards
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 PremiumRewardVendorId;

	// The vendor id of the battlepasses purchases, so pricing can be looked up
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 PurchaseVendorId;

	// Loot table item id of the battle pass itself for purchasing
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRH_LootId PassPurchaseLootId;

	// Loot table item id of the battle pass level for purchasing
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRH_LootId LevelPurchaseLootId;

	// Loot table item id that the instance will reward progress to the player with
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRH_LootId MatchProgressLootId;

	// Gets the current level the player is in their pass
	UFUNCTION(BlueprintCallable, Category = "Battlepass", meta = (DisplayName = "Get Current Level", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_GetCurrentLevel(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountDynamicDelegate& Delegate) { GetCurrentLevel(PlayerInfo, Delegate); }
	void GetCurrentLevel(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate = FRH_GetInventoryCountBlock());

	// Gets the total number of levels for the given pass
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "Battlepass")
	int32 GetLevelCount(const UObject* WorldContextObject) const;

	// Gets the total amount of Xp the player has earned so far in the pass
	UFUNCTION(BlueprintCallable, Category = "Battlepass", meta = (DisplayName = "Get Total Xp Progress", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_GetTotalXpProgress(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountDynamicDelegate& Delegate) { GetTotalXpProgress(PlayerInfo, Delegate); }
	void GetTotalXpProgress(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate = FRH_GetInventoryCountBlock());

	// Gets the level data for the pass, if it isn't cached, it will generate and cache it
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Battlepass")
	TArray<URHBattlepassLevel*> GetLevels(const UObject* WorldContextObject);

	// Returns the given level if it is in the cached levels
	UFUNCTION(BlueprintPure, Category = "Battlepass")
	URHBattlepassLevel* GetBattlepassLevel(int32 LevelNumber) const;

	// Gets the instant unlock rewards, if it isn't cached, it will generate and cache it
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "Battlepass")
	TArray<URHBattlepassRewardItem*> GetInstantUnlockRewards(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Battlepass")
	FORCEINLINE UIconInfo* GetFreeIconInfo() const { return FreeIconInfo; }

	UFUNCTION(BlueprintPure, Category = "Battlepass")
	FORCEINLINE UIconInfo* GetPremiumIconInfo() const { return PremiumIconInfo; }

	virtual void AppendRequiredLootTableIds(TArray<int32>& LootTableIds) override;

#if WITH_EDITOR
	virtual void GetPreloadDependencies(TArray<UObject*>& OutDeps) override;
#endif

protected:

	URHStoreItemHelper* GetStoreItemHelper(const UObject* WorldContextObject);

	// Flags the battlepass as fully loaded
	void RewardAssetsFullyLoaded();
	void InstantUnlocksAssetsFullyLoaded();

	UPROPERTY(EditDefaultsOnly)
	int32 XpTableId;

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Battlepass", meta = (DisplayName = "Free Icon"))
	UIconInfo* FreeIconInfo;

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Battlepass", meta = (DisplayName = "Premium Icon"))
	UIconInfo* PremiumIconInfo;

	UPROPERTY(Transient)
	TArray<URHBattlepassLevel*> CachedBattlepassLevels;

	UPROPERTY(Transient)
	TArray<URHBattlepassRewardItem*> CachedInstantUnlocks;

	UPROPERTY(BlueprintReadOnly)
	bool RewardsFullyLoaded;

	UPROPERTY(BlueprintReadOnly)
	bool InstantUnlocksFullyLoaded;
};
