#pragma once
#include "Online/CoreOnline.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Inventory/RHLootBox.h"
#include "RHLootBoxSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLootBoxOpenStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLootBoxOpenFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLootBoxLeave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLootBoxOpenSequenceCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDisplayLootBoxIntro, URHLootBox*, LootBox);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDisplayLootBoxIntroAndOpen, URHLootBox*, LootBox);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLootBoxContentsReceived, const TArray<UPlatformInventoryItem*>&, AcquiredItems);

class URHGameInstance;

UENUM()
enum ELootBoxContentsCategories
{
	LootBoxContents_All,
	LootBoxContents_Avatars,
	LootBoxContents_Banners,
	LootBoxContents_Titles,
	LootBoxContents_Borders,
	LootBoxContents_Other,
};

UCLASS(Config = Game)
class RALLYHERESTART_API URHLootBoxDetails : public UObject
{
	GENERATED_BODY()

public:
	// The loot box the added details are about
	UPROPERTY(BlueprintReadOnly, Category = "Loot Box | Contents")
	URHStoreItem* LootBox;

	// The contents broken down of that loot box
	UPROPERTY(BlueprintReadOnly, Category = "Loot Box | Contents")
	TArray<URHLootBoxContents*> Contents;
};

UCLASS(Config = Game)
class RALLYHERESTART_API URHLootBoxContents : public UObject
{
	GENERATED_BODY()

public:
	// The Loot Table ID of the vendor with the given contents
	UPROPERTY(BlueprintReadOnly, Category = "Loot Box | Contents")
	int32 LootTableId;

	// Array of store items in the loot table
	UPROPERTY(BlueprintReadOnly, Category = "Loot Box | Contents")
	TArray<TSoftObjectPtr<URHStoreItem>> BundleContents;

	// Can't nest a TArray in here with it being a UProperty, but the main contents array has all, which should cover these in memory.
	TMap<ELootBoxContentsCategories, TArray<TSoftObjectPtr<URHStoreItem>>> ContentsBySubcategory;

	// Returns formatted contents filter options sorted
	UFUNCTION()
	TArray<FText> GetContentsFilterOptions();

	TArray<TSoftObjectPtr<URHStoreItem>> GetContentsByFilterIndex(int32 Index);
};

UCLASS(Config = Game)
class RALLYHERESTART_API URHLootBoxSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// When a player logs on get any data needed for loot boxes
	void OnLoginPlayerChanged(ULocalPlayer* LocalPlayer);

	UFUNCTION()
	void OnStoreVendorsLoaded(bool bSuccess);

	void CallOnLootBoxOpenStarted();
	void CallOnLootBoxOpenFailed();
	void CallOnLootBoxContentsReceived(TArray<UPlatformInventoryItem*>& Items);
	void CallOnDisplayLootBoxIntro(URHLootBox* LootBox);
	void CallOnDisplayLootBoxIntroAndOpen(URHLootBox* LootBox);
	void CallOnLootBoxLeave();

	UFUNCTION(BlueprintCallable)
	void CallOnLootBoxOpeningSequenceComplete();

	// Returns if the given item is from a loot boxes
	bool IsFromLootBox(URHStoreItem* StoreItem);

	UFUNCTION(BlueprintPure)
	static FText GetContentCategoryName(ELootBoxContentsCategories Category);

	// Returns the details of a given loot box
	URHLootBoxDetails* GetLootBoxDetails(URHStoreItem* LootBox);

    // Delegates for loot box sequences

	// Called when a player clicks the open button
	UPROPERTY(BlueprintAssignable, Category = "Loot Box Manager")
	FOnLootBoxOpenStarted OnLootBoxOpenStarted;

	// Called if the player never gets a response back from the server on an open
	UPROPERTY(BlueprintAssignable, Category = "Loot Box Manager")
	FOnLootBoxOpenFailed OnLootBoxOpenFailed;

	// Called when the player gets the order of items from opening a loot box
	UPROPERTY(BlueprintAssignable, Category = "Loot Box Manager")
	FOnLootBoxContentsReceived OnLootBoxContentsReceived;

	// Called when the player is viewing their loot boxes and wants to see the 3D model for each
	UPROPERTY(BlueprintAssignable, Category = "Loot Box Manager")
	FDisplayLootBoxIntro OnDisplayLootBoxIntro;

	// Called when we need to drop the loot box in and then start the open sequence immediately
	UPROPERTY(BlueprintAssignable, Category = "Loot Bpx Manager")
	FDisplayLootBoxIntroAndOpen OnDisplayLootBoxIntroAndOpen;

	// Called when the player leaves the loot box scene
	UPROPERTY(BlueprintAssignable, Category = "Loot Box Manager")
	FOnLootBoxLeave OnLootBoxLeave;

	// Called after the animation of contents being opened is completed
	UPROPERTY(BlueprintAssignable, Category = "Loot Box Manager")
	FOnLootBoxOpenSequenceCompleted OnLootBoxOpenSequenceCompleted;

	UPROPERTY()
	TArray<FRH_LootId> LootBoxLootIds;

protected:

	// Returns the subcategory for the item based on what it is
	ELootBoxContentsCategories GetSubCategoryForItem(URHStoreItem* StoreItem) const;

	UPROPERTY(Transient)
	TObjectPtr<URHStoreSubsystem> StoreSubsystem;

	bool HasRequestedVendors;

	UPROPERTY(Transient)
	TMap<FRH_LootId, URHLootBoxDetails*> UnopenedLootBoxIdToContents;

	UPROPERTY(Config)
	int32 LootBoxsVendorId;

	UPROPERTY(Config)
	int32 LootBoxsRedemptionVendorId;
};