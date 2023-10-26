#pragma once
#include "Online.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RH_CatalogSubsystem.h"
#include "RH_PlayerInventory.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "Inventory/RHCurrency.h"
#include "RHStoreSubsystem.generated.h"

class URHStoreItem;

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FRH_GetStoreItemsDynamicDelegate, TArray<URHStoreItem*>, StoreItems);
DECLARE_DELEGATE_OneParam(FRH_GetStoreItemsDelegate, TArray<URHStoreItem*>);
DECLARE_RH_DELEGATE_BLOCK(FRH_GetStoreItemsBlock, FRH_GetStoreItemsDelegate, FRH_GetStoreItemsDynamicDelegate, TArray<URHStoreItem*>);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FRH_GetStoreItemDynamicDelegate, URHStoreItem*, StoreItem);
DECLARE_DELEGATE_OneParam(FRH_GetStoreItemDelegate, URHStoreItem*);
DECLARE_RH_DELEGATE_BLOCK(FRH_GetStoreItemBlock, FRH_GetStoreItemDelegate, FRH_GetStoreItemDynamicDelegate, URHStoreItem*);

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_TwoParams(FRH_GetAffordableItemsInVendorDynamicDelegate, const TArray<URHStoreItem*>&, CurrencyItems, const TArray<URHStoreItem*>&, PurchaseItems);
DECLARE_DELEGATE_TwoParams(FRH_GetAffordableItemsInVendorDelegate, const TArray<URHStoreItem*>&, const TArray<URHStoreItem*>&);
DECLARE_RH_DELEGATE_BLOCK(FRH_GetAffordableItemsInVendorBlock, FRH_GetAffordableItemsInVendorDelegate, FRH_GetAffordableItemsInVendorDynamicDelegate, const TArray<URHStoreItem*>&, const TArray<URHStoreItem*>&);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRHPurchaseStoreItem, URHStoreItem*, Item, URHStoreItemPrice*, Price);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRHPurchasePortalItem, URHStoreItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRHNotEnoughCurrency, URHStorePurchaseRequest*, PurchaseRequest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHOnReceiveXpTables);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHOnReceivePricePoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHOnPortalOffersReceived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHOnReceivePendingPurchase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHOnPurchaseSubmitted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHStoreItemPriceDirty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHStoreVendorsReady);
DECLARE_DELEGATE_TwoParams(FRHOnReceivePendingPurchaseNative, const URH_PlayerInfo* PlayerInfo, const FRHAPI_PlayerOrder& PlayerOrderResult);

namespace ConstItemTypeIds
{
	const int32 Container = 1634;
	const int32 Currency = 10247;
}

namespace RecipeItemTypeIds
{
	const int32 None = 0;
	const int32 Produced = 16971;
	const int32 Consumed_Optional = 16972;
	const int32 Consumed_Required = 16973;
	const int32 Required = 16974;
	const int32 Blocker = 16975;
	const int32 Assigned = 17483;
}

namespace CollectionNames
{
	const FName ActivityCollectionName = FName(TEXT("ItemCollection.Activity"));
	const FName AvatarCollectionName = FName(TEXT("ItemCollection.Avatar"));
	const FName BannerCollectionName = FName(TEXT("ItemCollection.Banner"));
	const FName BattlepassCollectionName = FName(TEXT("ItemCollection.Battlepass"));
	const FName BorderCollectionName = FName(TEXT("ItemCollection.Border"));
	const FName CurrencyCollectionName = FName(TEXT("ItemCollection.Currency"));
	const FName EventCollectionName = FName(TEXT("ItemCollection.Event"));
	const FName LootBoxCollectionName = FName(TEXT("ItemCollection.LootBox"));
	const FName TitleCollectionName = FName(TEXT("ItemCollection.Title"));
	const FName ProgressionCollectionName = FName(TEXT("ItemCollection.Progression"));
}

UENUM()
enum EStoreSectionTypes
{
    AccountCosmetics,
	Bundles,
	PortalOffers
};

UENUM()
enum EStoreItemCollectionMode
{
	StoreItems,
	BlockedItems,
	RefundedItems
};

USTRUCT()
struct FStorePriceKey
{
	GENERATED_BODY()

public:
	FGuid PricePointGuid;

	FGuid PreSalePricePointGuid;

	FStorePriceKey() { };
	FStorePriceKey(const FGuid& PPGuid, const FGuid& PSPPGuid) : PricePointGuid(PPGuid), PreSalePricePointGuid(PSPPGuid)
	{
	};

	bool operator== (const FStorePriceKey& Other) const
	{
		return PricePointGuid == Other.PricePointGuid &&
			PreSalePricePointGuid == Other.PreSalePricePointGuid;
	}
};

inline uint32 GetTypeHash(const FStorePriceKey& A)
{
	return HashCombine(GetTypeHash(A.PricePointGuid), GetTypeHash(A.PreSalePricePointGuid));
}

USTRUCT()
struct RALLYHERESTART_API FStoreItemWithTrueSort
{
	GENERATED_BODY()

public:
	// Item within a loot table
	UPROPERTY(Transient)
	URHStoreItem* StoreItem;

	// List of sort orders as you dig down into a container, for full sorting of an item
	TArray<int32> SortOrders;

	FStoreItemWithTrueSort() : StoreItem(nullptr) {}
	FStoreItemWithTrueSort(URHStoreItem* InStoreItem, TArray<int32> InSort) : StoreItem(InStoreItem), SortOrders(InSort) {}
};

// Small class for items of type coupon to be created in editor to put names/etc. on them.  Possibly more data later
UCLASS()
class RALLYHERESTART_API URH_Coupon final : public UPlatformInventoryItem
{
	GENERATED_BODY()
};

UCLASS(Config = Game, BlueprintType)
class RALLYHERESTART_API URHStorePurchaseRequest : public UObject
{
    GENERATED_BODY()

public:
    // The loot table item id of the item, set by the creation of the request.
    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Purchase Request")
    FRH_LootId LootTableItemId;

    // The vendor id the item belongs to, set by the creation of the request.
    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Purchase Request")
    int32 VendorId;

    // The price displayed in the UI, used to make sure the player is paying what they see.
    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
    int32 PriceInUI;

    // The currency type displayed in the UI.
    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
    UPlatformInventoryItem* CurrencyType;

    // The quantity the player is purchasing, defaults to 1.
    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
    int32 Quantity;

    // The location the player is making the purchase for stats tracking.
    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
    int32 LocationId;

	// This ID can be set for a given purchase to give more context of where it came from
	UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
	FString ExternalTransactionId;

    // Id of a coupon being used for the purchase
    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
    FRH_ItemId CouponId;

    // Should we skip validating if the client has enough currency to complete this purchase request
    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
    bool SkipCurrencyAmountValidation;

	// The player the purchase request is for
	UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Purchase Request")
	const URH_PlayerInfo* RequestingPlayerInfo;

    // Reference to the Store Subsystem that created us
    UPROPERTY()
    TWeakObjectPtr<class URHStoreSubsystem> pStoreSubsystem;

    // Submits the Purchase Request
    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Purchase Request", meta = (DisplayName = "Submit Purchase Request", AutoCreateRefTerm = "Delegate"))
    void SubmitPurchaseRequest(const FRH_CatalogCallDynamicDelegate& Delegate);

    // Checks if the Purchase Request has all of the required values to actually purchase something
    virtual bool IsValid()
    {
        return LootTableItemId.IsValid() &&
               VendorId > 0              &&
               PriceInUI > 0             &&
               Quantity > 0              &&
			   RequestingPlayerInfo != nullptr &&
               CurrencyType != nullptr;
    };
};

UCLASS(Config = Game, BlueprintType)
class RALLYHERESTART_API URHPortalOffer : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
    FString SKU;

	UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
	float PreSaleCost;

	UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
	FText DisplayPreSaleCost;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
    float Cost;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
    FText DisplayCost;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
    FText CurrencyCode;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
    FText Name;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
    FText Desc;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Real Money Store Item")
    FText ShortDesc;

	// Returns the calculated discount percentage of the portal offer
	UFUNCTION(BlueprintPure, Category = "Platform UMG | Real Money Store Item")
	int32 GetDiscountPercentage() const;
};

UCLASS(Config = Game, BlueprintType)
class RALLYHERESTART_API URHStoreItemPrice : public UObject
{
    GENERATED_BODY()

public:
    // If an item is put on sale, this is the price it was being sold for before, allowing us to show slashed out prices or other UI treatments for discounts
    UPROPERTY(BlueprintReadOnly, Category = "Store Item Price")
    int32 PreSalePrice;

    // This is the current price the item is being sold for
    UPROPERTY()
    int32 Price;

    // This is the item type that is consumed when purchasing this item
    UPROPERTY(BlueprintReadOnly, Category = "Store Item Price")
    TSoftObjectPtr<UPlatformInventoryItem> CurrencyType;

    // Reference to the Store Subsystem that created us
    UPROPERTY()
    TWeakObjectPtr<class URHStoreSubsystem> pStoreSubsystem;

	// Returns the computed price of the offer with the coupon
	UFUNCTION(BlueprintPure, Category = "Store Item Price")
	int32 GetPriceWithCoupon(URHStoreItem* Coupon) const;

	// Returns if the player has enough currency to purchase this price
    UFUNCTION(BlueprintCallable, Category = "Store Item Price", meta = (DisplayName = "Can Afford", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_CanAfford(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateDynamicDelegate& Delegate, int32 Quantity = 1, URHStoreItem* Coupon = nullptr) { CanAfford(PlayerInfo, Quantity, Coupon, Delegate); }
	void CanAfford(const URH_PlayerInfo* PlayerInfo, int32 Quantity = 1, URHStoreItem* Coupon = nullptr, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());

    // Returns the discount percentage of the given price rounded to the nearest percent
    UFUNCTION(BlueprintPure, Category = "Store Item Price")
    int32 GetDiscountPercentage() const;
};

UCLASS(Config = Game, BlueprintType)
class RALLYHERESTART_API URHStoreItem : public UObject
{
    GENERATED_BODY()

public:
    // Initializes the Store Item, setting its reference back to the Store Subsystem and the item is represents
    void Initialize(URHStoreSubsystem* pStoreSubsystem, const FRHAPI_Loot& LootItem);

	 // Returns the reference to the uasset version of the item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    TSoftObjectPtr<UPlatformInventoryItem> GetInventoryItem() const;

	// Returns the catalog version of the item
	URH_CatalogItem* GetCatalogItem() const;

	// Returns the catalog vendor item version of the item
	bool GetCatalogVendorItem(FRHAPI_Loot& LootItem) const;

    // Calculates if needed then, returns the price of the item for each currency type the item is priced with
    UFUNCTION(BlueprintPure, Category = "Store Item")
    TArray<URHStoreItemPrice*> GetPrices();

    // Calculates if needed then, returns the price of the item for a specific currency type
    UFUNCTION(BlueprintPure, Category = "Store Item")
    URHStoreItemPrice* GetPrice(TSoftObjectPtr<UPlatformInventoryItem> nCurrencyType);

    // Returns if the player has enough currency to purchase this price
    UFUNCTION(BlueprintCallable, Category = "Store Item", meta = (DisplayName = "Can Afford", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_CanAfford(const URH_PlayerInfo* PlayerInfo, const URHStoreItemPrice* Price, const FRH_GetInventoryStateDynamicDelegate& Delegate, int32 Quantity = 1) { CanAfford(PlayerInfo, Price, Quantity, Delegate); }
	void CanAfford(const URH_PlayerInfo* PlayerInfo, const URHStoreItemPrice* Price, int32 Quantity = 1, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());

	// Gets any coupons associated with a given price the player owns
	UFUNCTION(BlueprintCallable, Category = "Store Item", meta = (DisplayName = "Get Coupons For Price", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_GetCouponsForPrice(const URHStoreItemPrice* Price, const URH_PlayerInfo* PlayerInfo, const FRH_GetStoreItemsDynamicDelegate& Delegate) { GetCouponsForPrice(Price, PlayerInfo, Delegate); }
	void GetCouponsForPrice(const URHStoreItemPrice* Price, const URH_PlayerInfo* PlayerInfo, const FRH_GetStoreItemsBlock& Delegate = FRH_GetStoreItemsBlock());

	// Gets the best coupon for a given price the player owns
	UFUNCTION(BlueprintCallable, Category = "Store Item", meta = (DisplayName = "Get Best Coupon For Price", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_GetBestCouponForPrice(const URHStoreItemPrice* Price, const URH_PlayerInfo* PlayerInfo, const FRH_GetStoreItemDynamicDelegate& Delegate) { GetBestCouponForPrice(Price, PlayerInfo, Delegate); }
	void GetBestCouponForPrice(const URHStoreItemPrice* Price, const URH_PlayerInfo* PlayerInfo, const FRH_GetStoreItemBlock& Delegate = FRH_GetStoreItemBlock());

    // Calculates the best discounted price and returns it to the closest whole number rounded down
    UFUNCTION(BlueprintPure, Category = "Store Item")
    int32 GetBestDiscount();

	// Returns if the item is on sale
	UFUNCTION(BlueprintCallable, Category = "Store Item")
    bool IsOnSale();

	UFUNCTION(BlueprintPure, Category = "Store Item")
	FRH_ItemId GetItemId() const;

    UFUNCTION(BlueprintPure, Category = "Store Item")
    const FRH_LootId& GetLootId() const;

    UFUNCTION(BlueprintPure, Category = "Store Item")
    int32 GetVendorId() const;

    UFUNCTION(BlueprintPure, Category = "Store Item")
    int32 GetSortOrder() const;

    // Checks if the player owns the item
	UFUNCTION(BlueprintCallable, Category = "Store Item", meta = (DisplayName = "Is Owned", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_IsOwned(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateDynamicDelegate& Delegate) { IsOwned(PlayerInfo, Delegate); }
	void IsOwned(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());
	
	// Checks if the player can own more copies of an item
	UFUNCTION(BlueprintCallable, Category = "Store Item", meta = (DisplayName = "Can Own More", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_CanOwnMore(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateDynamicDelegate& Delegate) { CanOwnMore(PlayerInfo, Delegate); }
	void CanOwnMore(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());
	
    // Checks if the player is renting the item
	UFUNCTION(BlueprintCallable, Category = "Store Item", meta = (DisplayName = "Is Rented", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_IsRented(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateDynamicDelegate& Delegate) { IsRented(PlayerInfo, Delegate); }
	void IsRented(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());

    // Checks if the item is Active and should be available to be used in our game
    UFUNCTION(BlueprintPure, Category = "Store Item")
    bool IsActive() const;

    // Returns the number of the item the player owns
	UFUNCTION(BlueprintCallable, Category = "Store Item", meta = (DisplayName = "Get Quantity Owned", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_GetQuantityOwned(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountDynamicDelegate& Delegate) { GetQuantityOwned(PlayerInfo, Delegate); }
	void GetQuantityOwned(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate);

    // Returns the quantity of the item being purchased
    UFUNCTION(BlueprintPure, Category = "Store Item")
    int32 GetLootQuantity() const;

    // Returns the bundle id of the item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    int32 GetBundleId() const;

	// Returns if the item is a Bundle and that it contains the given item id
	UFUNCTION(BlueprintPure, Category = "Store Item")
	bool BundleContainsItemId(const FRH_ItemId& nItemId, bool bSearchSubContainers) const;

    // Returns the top level items within a bundle
    UFUNCTION(BlueprintPure, Category = "Store Item")
    bool GetBundledContents(TArray<URHStoreItem*>& ContainedItems) const;

	// Returns the path for the items icon
	UFUNCTION(BlueprintPure, Category = "Store Item")
	UIconInfo* GetIconInfo() const;

    // Gets the formatted display name for the item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    FText GetFormattedNameDisplay(int32 ExternalQuantity = 1) const;

    // Returns the name of the item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    FText GetName() const;

    // Gets the formatted display description for the item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    FText GetFormattedDescDisplay() const;

    // Returns the description of the item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    FText GetDescription() const;

    // Returns the inventory operation of the loot table item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    ERHAPI_InventoryOperation GetInventoryOperation() const;

    // Returns the UI Hint provided by pricing schedule for the item
    UFUNCTION(BlueprintPure, Category = "Store Item")
    int32 GetUIHint() const;

	bool GetSku(FString& SKU);

	// Gets the DLC a given DLC Voucher is used for
	UFUNCTION(BlueprintCallable)
	URHStoreItem* GetDLCForVoucher();

	// Returns if the item is one that the game cares to display to the user
    UFUNCTION(BlueprintPure, Category = "Store Item")
    bool ShouldDisplayToUser() const;

	// Sets the portal offer for the given item
	void SetPortalOffer(URHPortalOffer* InPortalOffer) { PortalOffer = InPortalOffer; }

    // Returns the external portal offer for this item
    UFUNCTION(BlueprintPure, Category = "Store Item")
	URHPortalOffer* GetPortalOffer() const { return PortalOffer; }

    // Returns if this item has an external portal offer to purchase it
    UFUNCTION(BlueprintPure, Category = "Store Item")
    bool HasPortalOffer() const { return PortalOffer != nullptr; }

    // Used to purchase the given item at a given price point, this should result in a purchase confirmation window to prevent accidental purchases
    UFUNCTION(BlueprintCallable, Category = "Store Item")
    void UIX_ShowPurchaseConfirmation(URHStoreItemPrice* pPrice);

    // Returns a partially filled out purchase request for the item, that requires the UI to provide the final details to prevent improper purchases
    UFUNCTION(BlueprintCallable, Category = "Store Item")
    URHStorePurchaseRequest* GetPurchaseRequest();

    // Attempts to direct the user to the portals store to purchase the item
    UFUNCTION(BlueprintCallable, Category = "Store Item")
    void PurchaseFromPortal();

    // Completes sending the player to the external portal after they confirm they wish to leave our application
    UFUNCTION(BlueprintCallable, Category = "Store Item")
    void ConfirmGotoPortalOffer();

    // Delegate that can be listened to by UI to know when prices on an item have possibly changed
    UPROPERTY(BlueprintAssignable, Category = "Store Item")
    FRHStoreItemPriceDirty OnPriceSetDirty;

	// Delegate that fires whenever a portal purchase has its entitlements requested
	UPROPERTY(BlueprintAssignable, Category = "Item Helper")
	FRHOnPurchaseSubmitted OnPortalPurchaseSubmitted;

protected:

    // Callback from purchasing from portal
    void PortalPurchaseComplete(const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt);

	// Gets the prices for the item off the StoreSubsystem
	bool GetPriceData(TArray<URHStoreItemPrice*>& Prices);

    // The loot item id of this store item
    FRH_LootId LootId;

    // The vendor id that this loot item belongs to
    int64 VendorId;

    // Reference to the Store Item Helper that created us
    UPROPERTY()
    TWeakObjectPtr<class URHStoreSubsystem> pStoreSubsystem;

    // Reference to the platform inventory item associated with the Store Item
    UPROPERTY()
    TSoftObjectPtr<UPlatformInventoryItem> InventoryItem;

	// The Store offer associated with this item
	UPROPERTY()
	URHPortalOffer* PortalOffer;

	UPROPERTY(Transient)
	TArray<URH_GetOwnedCouponsAsyncTaskHelper*> GetOwnedCouponsTaskHelpers;

private:

	void OnAsyncItemLoadedForOwnership(TSoftObjectPtr<UPlatformInventoryItem> InvItem, const URH_PlayerInfo* PlayerInfo, FRH_GetInventoryStateBlock Delegate);
	void OnAsyncItemLoadedForOwnMore(TSoftObjectPtr<UPlatformInventoryItem> InvItem, const URH_PlayerInfo* PlayerInfo, FRH_GetInventoryStateBlock Delegate);
	void OnAsyncItemLoadedForRental(TSoftObjectPtr<UPlatformInventoryItem> InvItem, const URH_PlayerInfo* PlayerInfo, FRH_GetInventoryStateBlock Delegate);
	void OnAsyncItemLoadedForQuantityOwned(TSoftObjectPtr<UPlatformInventoryItem> InvItem, const URH_PlayerInfo* PlayerInfo, FRH_GetInventoryCountBlock Delegate);
};

class URH_PurchaseAsyncTaskHelper;

USTRUCT(BlueprintType)
struct FAccountConsumableDetails
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	UPlatformInventoryItem* Item;

	UPROPERTY(BlueprintReadOnly)
	int32 QuantityOwned;

	FAccountConsumableDetails() : Item(nullptr), QuantityOwned(0) {};
	FAccountConsumableDetails(UPlatformInventoryItem* Item, int32 Quantity) : Item(Item), QuantityOwned(Quantity) {};
};

UCLASS(Config = Game, BlueprintType)
class RALLYHERESTART_API URHStoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

    // Redeems a DLC Voucher for the DLC Product it is associated with
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Redeem DLC Voucher", AutoCreateRefTerm = "Delegate"))
    void RedeemDLCVoucher(URHStoreItem* DLCVoucher, URH_PlayerInfo* PlayerInfo, const FRH_CatalogCallDynamicDelegate& Delegate);

    // Gets the DLC a given DLC Voucher is used for
    URHStoreItem* GetDLCForVoucher(const URHStoreItem* DLCVoucher);

    // Request the store vendors contents
    void GetUpdatedStoreContents();	

	// Returns a list of all affordable items in the voucher redemption vendor, and the vouchers used to purchase them
	void GetAffordableVoucherItems(URH_PlayerInfo* PlayerInfo, FRH_GetAffordableItemsInVendorBlock Delegate = FRH_GetAffordableItemsInVendorBlock());

	// Returns a list of all redeemable items and the items used to purchase them
	void GetRedeemableItemsInVendor(URH_PlayerInfo* PlayerInfo, int32 CurrencyVendorId, int32 RedemptionVendorId, FRH_GetAffordableItemsInVendorBlock Delegate = FRH_GetAffordableItemsInVendorBlock());

	// Returns if a given voucher item has a item that it can be redeemd for currently
	bool CanRedeemVoucher(URHStoreItem* VoucherItem);

	int32 GetStoreVendorId() const { return StoreVendorId; }

	int32 GetGameCurrencyVendorId() const { return GameCurrencyVendorId; }

	UFUNCTION(BlueprintPure)
	URHCurrency* GetCurrencyItem(const FRH_ItemId& ItemId) const;

	UFUNCTION(BlueprintPure)
	URHCurrency* GetPremiumCurrencyItem() const { return PremiumCurrencyItem; }

	UFUNCTION(BlueprintPure)
	URHCurrency* GetFreeCurrencyItem() const { return FreeCurrencyItem; }

	// Returns the Store Item for a given loot id, if it is not cached, it will create and return it.
	UFUNCTION(BlueprintPure, Category = "Item Helper")
    URHStoreItem* GetStoreItem(const FRH_LootId& LootId);

    // Returns the Store Item for a given Vendor Item, if it is not cached, it will create and return it.
    URHStoreItem* GetStoreItem(const FRHAPI_Loot& LootItem);

    // Returns if the player has enough currency to make the supplied purchase request
    void HasEnoughCurrency(URHStorePurchaseRequest* pPurchaseRequest, const FRH_CatalogCallBlock& Delegate = FRH_CatalogCallBlock());

    // Gets the current currency value a player has of a currency type
    void GetCurrencyOwned(TSoftObjectPtr<UPlatformInventoryItem> CurrencyType, const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate = FRH_GetInventoryCountBlock());
    void GetCurrencyOwned(UPlatformInventoryItem* pPurchaseRequest, const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate = FRH_GetInventoryCountBlock());

    // Returns the Store Item for a specific loot id within a specific vendor
    UFUNCTION(BlueprintPure, Category = "Item Helper")
    URHStoreItem* GetStoreItemForVendor(int32 nVendorId, const FRH_LootId& nLootItemId);

    // Returns all Store Items that are in a given vendor
    UFUNCTION(BlueprintPure, Category = "Item Helper")
    TArray<URHStoreItem*> GetStoreItemsForVendor(int32 nVendorId, bool bIncludeInactiveItems, bool bSearchSubContainers);

    UFUNCTION(BlueprintCallable, Category = "Item Helper")
    TArray<URHStoreItem*> GetStoreItemsAndQuantitiesForVendor(int32 nVendorId, bool bIncludeInactiveItems, bool bSearchSubContainers, TMap<FRH_LootId, int32>& QuantityMap, int32 ExternalQuantity = 1);

    TArray<FRH_LootId> GetLootIdsForVendor(int32 nVendorId, bool bIncludeInctiveItems, bool bSearchSubContainers);

    // Looks up the PricePoints for the GUID, and creates it if needed
    bool GetPriceForGUID(FStorePriceKey Guid, TArray<URHStoreItemPrice*>& Prices);

    // Makes a request to the backend to retrieve vendor data for a given vendor Ids
	UFUNCTION(BlueprintCallable, Category = "Item Helper", meta = (DisplayName = "Request Vendor Data", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_RequestVendorData(TArray<int32> VendorIds, const FRH_CatalogCallDynamicDelegate& Delegate) { RequestVendorData(VendorIds, Delegate); }
	void RequestVendorData(TArray<int32> VendorIds, FRH_CatalogCallBlock Delegate = FRH_CatalogCallBlock());

    // Reports to the Online Subsystem that an in game store that can sell portal offers has been entered
    UFUNCTION(BlueprintCallable, Category = "Item Helper")
    void EnterInGameStoreUI();

    // Reports to the Online Subsystem that an in game store that can sell portal offers has been exited
    UFUNCTION(BlueprintCallable, Category = "Item Helper")
    void ExitInGameStoreUI();

    // Checks if we've failed to fetch any platform offers and if so, it displays a platform message
    UFUNCTION(BlueprintCallable, Category = "Item Helper")
    bool CheckEmptyInGameStore(const class UObject* WorldContextObject);

	// Called when processing of platform entitlements is complete to allow for monitoring of resulting orders
	void EntitlementProcessingComplete(bool bSuccessful, FRHAPI_PlatformEntitlementProcessResult Result);

	// Called when entitlement result is returned from core
	UFUNCTION()
	void OnEntitlementResult(const TArray<FRHAPI_PlayerOrder>& OrderResults, const URH_PlayerInfo* PlayerInfo);

	// Returns the Item based on the given ItemId
	TSoftObjectPtr<UPlatformInventoryItem> GetInventoryItem(const FRH_ItemId& ItemId) const;

    // Delegate that fires whenever a player is attempting to purchase an item
    UPROPERTY(BlueprintAssignable, Category = "Item Helper")
    FRHPurchaseStoreItem OnPurchaseItem;

	// Delegate that fires whenever a player is trying to purchase a portal offer that needs an intermediary process
	UPROPERTY(BlueprintAssignable, Category = "Item Helper")
	FRHPurchasePortalItem OnPurchasePortalItem;

    // Delegate that fires if a player does not have enough currency to complete a purchase
    UPROPERTY(BlueprintAssignable, Category = "Item Helper")
    FRHNotEnoughCurrency OnNotEnoughCurrency;

    // Delegate that fires when xp tables are retrieved from the backend
    UPROPERTY(BlueprintAssignable, Category = "Item Helper")
    FRHOnReceiveXpTables OnReceiveXpTables;

	// Delegate that fires when price points are retrieved from the backend
	UPROPERTY(BlueprintAssignable, Category = "Item Helper")
	FRHOnReceivePricePoints OnReceivePricePoints;

    // Delegate that fires when portal offers have been retrieved from the backend and portal
	UPROPERTY(BlueprintAssignable, Category = "Item Helper")
	FRHOnPortalOffersReceived OnPortalOffersReceived;

    // Delegate that fires whenever a pending purchase is completed
    UPROPERTY(BlueprintAssignable, Category = "Item Helper")
    FRHOnReceivePendingPurchase OnPendingPurchaseReceived;
	FRHOnReceivePendingPurchaseNative OnPendingPurchaseReceivedNative;

    // Delegate that fires whenever a purchase is submitted to the core
    UPROPERTY(BlueprintAssignable, Category = "Item Helper")
    FRHOnPurchaseSubmitted OnPurchaseSubmitted;

    // Called to complete a purchase from the purchase confirmation screen
    UFUNCTION(BlueprintCallable, Category = "Item Helper", meta = (DisplayName = "Complete Purchase Item", AutoCreateRefTerm = "Delegate"))
    void UIX_CompletePurchaseItem(URHStorePurchaseRequest* PurchaseRequest, const FRH_CatalogCallDynamicDelegate& Delegate);

    // Takes all purchase requests and submits a final purchase to the server
    void SubmitFinalPurchase(TArray<URHStorePurchaseRequest*> PurchaseRequests, const FRH_CatalogCallBlock& Delegate = FRH_CatalogCallBlock());

    template <typename T>
    bool GetInventoryItemByLootId(const FRH_LootId& LootTableItemId, int32 VendorId, bool RequireActive, T*& Item)
    {
        Item = nullptr;
        URHStoreItem* StoreItem = GetStoreItemForVendor(VendorId, LootTableItemId);

        if (StoreItem && (!RequireActive || StoreItem->IsActive()) && StoreItem->GetInventoryItem().IsValid())
        {
            Item = Cast<T>(StoreItem->GetInventoryItem().Get());
        }

        return (Item != nullptr);
    }

    UFUNCTION(BlueprintPure, Category = "Item Helper")
    bool AreXpTablesLoaded() { return XpTablesLoaded; }

	UFUNCTION(BlueprintPure, Category = "Item Helper")
	bool AreInventoryBucketUseRuleSetsLoaded() { return InventoryBucketUseRuleSetsLoaded; }

	UFUNCTION(BlueprintPure, Category = "Item Helper")
	bool ArePricePointsLoaded() { return PricePointsLoaded; }

	UFUNCTION(BlueprintPure, Category = "Item Helper")
	bool ArePortalOffersLoaded() { return PortalOffersLoaded; }

    // Returns the Xp Table for a given xp table id, if it is not cached, it will create and return it.
    UFUNCTION(BlueprintPure, Category = "Item Helper")
    bool GetXpTable(int32 XpTableId, FRHAPI_XpTable& XpTable);

    // Returns if the portal the player is connected to can have portal offers
    UFUNCTION(BlueprintPure, Category = "Item Helper")
    bool DoesPortalHaveOffers() const { return GetPortalOffersVendorId() != INDEX_NONE; }

    // Sends off a list of offer ids to request from the portal
    bool QueryPortalOffers();

	// Callback if the portal query was delayed due to needing to load the inventory items
	void PortalOffersItemsFullyLoaded();

    // Loads all of the portal offers into the Portal Offers Map once we have them back from the server
    void QueryPlatformStoreOffersComplete(bool bSuccess, const TArray<FUniqueOfferId>& offers, const FString& error);

    // Returns if there is currently a pending purchase
    UFUNCTION(BlueprintPure, Category = "Item Helper")
    bool HasPendingPurchase() const { return PendingPurchaseData.Num() > 0 || PurchaseTaskHelper != nullptr; }

    UFUNCTION(BlueprintPure, Category = "Item Helper")
    TArray<URHStorePurchaseRequest*> GetPendingPurchaseData() const { return PendingPurchaseData; }

	// Returns all of the coupons associated with the given store item
	void GetCouponsForItem(const URHStoreItem* StoreItem, TArray<FRHAPI_Loot>& Coupons) const;

	const FRH_ItemId& GetPremiumCurrencyItemId() const { return PremiumCurrencyItemId; }

	// Returns the vendor id of the portal offer store
	int32 GetPortalOffersVendorId() const { return PortalOffersVendorId; }

	static class IOnlineSubsystem* GetStoreOSS();

	URH_PlayerInfo* GetPlayerInfo(const FGuid& PlayerUuid) const;

	URH_CatalogSubsystem* GetRH_CatalogSubsystem() const;

	// Stores if we have received an initial store vendor
	UPROPERTY(BlueprintReadOnly)
	bool StoreVendorsLoaded;

	UPROPERTY(BlueprintAssignable)
	FRHStoreVendorsReady OnStoreItemsReady;

	UPROPERTY(Transient)
	TArray<URH_GetAllAffordableItemsHelper*> GetOwnershipTrackers;

protected:
	// Creates a StoreItem from a Catalog Vendor Item
	URHStoreItem* CreateStoreItem(const FRHAPI_Loot& LootItem);

	// Sorts price data based on any special rules
	void SortPriceData(TArray<URHStoreItemPrice*>& Prices);

	// Returns the vendor id of the voucher redemption loot table for the current portal
	int32 GetVoucherVendorId() const;

	virtual void GetAdditionalStoreVendors(TArray<int32>& StoreVendorsIds);

	// Checks if a purchase request is valid for a player
	void CheckPurchaseRequestValidity(URHStorePurchaseRequest* PurchaseRequest, const FRH_CatalogCallBlock& Delegate = FRH_CatalogCallBlock());

	// Generates all of the price data for a given price point tuple
	void GeneratePriceData(FStorePriceKey PricePoint, TArray<URHStoreItemPrice*>& Prices);

	// Builds out the coupons lookup map
	void ProcessCouponVendor();

	void GetRedeemableItemsInVendor_INTERNAL(URH_PlayerInfo* PlayerInfo, int32 CurrencyVendorId, int32 RedemptionVendorId, FRH_GetAffordableItemsInVendorBlock Delegate);

    UFUNCTION()
	void OnXpTablesUpdated(bool Success);

	UFUNCTION()
	void OnPricePointsUpdated(bool Success);

	UFUNCTION()
	void OnInventoryBucketUseRuleSetsUpdated(bool Success);

	void OnLoginPlayerChanged(ULocalPlayer* LocalPlayer);

	// Pending Purchase info, for display purposes if needed
    TArray<URHStorePurchaseRequest*> PendingPurchaseData;

    // List of Store Items based on Sku to look up on offer creation
    UPROPERTY(Transient)
    TMap<FString, URHStoreItem*> SkuToStoreItem;

    // References to all Store Items organized by Loot Id
    UPROPERTY(Transient)
    TMap<FRH_LootId, URHStoreItem*> StoreItems;

    // A map that resolves Price Point Guids to Price data used by the client
    TMap<FStorePriceKey, TArray<URHStoreItemPrice*>> PricePoints;

	// Map of loot table item ids to the coupons that are valid for them.
	TMultiMap<FRH_LootId, FRHAPI_Loot> CouponsForLootTableItems;

	UPROPERTY(config)
	FName StoreOSS;

    UPROPERTY(Transient)
    bool XpTablesLoaded;

	UPROPERTY(Transient)
	bool InventoryBucketUseRuleSetsLoaded;

	UPROPERTY(Transient)
	bool PricePointsLoaded;

	UPROPERTY(Transient)
	bool PortalOffersLoaded;

	// Stores if we have received an initial store vendor
	UPROPERTY(Transient, BlueprintReadOnly)
	bool BaseStoreVendorsLoaded;

    UPROPERTY(Transient)
    bool IsQueryingPortalOffers;

	UPROPERTY(Config)
	int32 PortalOffersVendorId;

	UPROPERTY(Config)
	int32 StoreVendorId;

	UPROPERTY(Config)
	int32 GameCurrencyVendorId;

	UPROPERTY(Config)
	int32 DailyRotationVendorId;

	UPROPERTY(Config)
	int32 CouponVendorId;

	UPROPERTY(Config)
	int32 EpicVoucherVendorId;

	UPROPERTY(Config)
	int32 ValveVoucherVendorId;

	UPROPERTY(Config)
	int32 NintendoVoucherVendorId;

	UPROPERTY(Config)
	int32 MicrosoftVoucherVendorId;

	UPROPERTY(Config)
	int32 SonyVoucherVendorId;

	// #RHTODO: These should probably be asset references paths not configed ids.
	UPROPERTY(BlueprintReadOnly, config)
	FRH_ItemId FreeCurrencyItemId;

	UPROPERTY(BlueprintReadOnly, config, Category = "Item Helper")
	FRH_ItemId PremiumCurrencyItemId;

	UPROPERTY(transient)
	URHCurrency* PremiumCurrencyItem;

	UPROPERTY(transient)
	URHCurrency* FreeCurrencyItem;

	UPROPERTY(Transient)
	URH_PurchaseAsyncTaskHelper* PurchaseTaskHelper;

	friend class URH_PurchaseAsyncTaskHelper;
};

UCLASS()
class URH_PurchaseAsyncTaskHelper : public UObject
{
	GENERATED_BODY()

public:
	void CheckPurchaseValidity();

	int32 RequestsCompleted;
	bool HasResponded;

	UPROPERTY()
	URH_CatalogSubsystem* CatalogSubsystem;

	UPROPERTY()
	URHStoreSubsystem* StoreSubsystem;

	FRH_CatalogCallDelegate OnCompleteDelegate;

	UPROPERTY(Transient)
	TArray<URHStorePurchaseRequest*> PurchaseRequests;
};

UCLASS()
class URH_GetOwnedCouponsAsyncTaskHelper : public UObject
{
	GENERATED_BODY()

public:
	void Start();
	bool IsCompleted() const { return RequestsCompleted >= ItemsToCheck.Num(); }

	int32 RequestsCompleted;
	FRH_GetStoreItemsDelegate OnCompleteDelegate;

	UPROPERTY()
	URH_PlayerInventory* PlayerInventory;

	UPROPERTY(Transient)
	TArray<URHStoreItem*> ItemsToCheck;

	UPROPERTY(Transient)
	TArray<URHStoreItem*> OwnedItems;

private:
	void OnOwnershipCheck(bool bOwned, URHStoreItem* CouponItem);
};

UCLASS()
class URH_GetAllAffordableItemsHelper : public UObject
{
	GENERATED_BODY()

public:
	void Start();
	bool IsCompleted() const { return RequestsCompleted >= ItemsToCheck.Num(); }

	FRH_GetAffordableItemsInVendorBlock OnCompleteDelegate;
	int32 RequestsCompleted;

	int32 CurrencyVendorId;

	UPROPERTY()
	TArray<URHStoreItem*> PurchaseItems;

	UPROPERTY()
	TArray<URHStoreItem*> CurrencyItems;

	UPROPERTY()
	URH_PlayerInfo* PlayerInfo;

	UPROPERTY()
	URH_CatalogSubsystem* CatalogSubsystem;

	UPROPERTY()
	URHStoreSubsystem* StoreSubsystem;

	UPROPERTY(Transient)
	TMap<URHStoreItem*, URHStoreItemPrice*> ItemsToCheck;

	UPROPERTY(Transient)
	TArray<URHStoreItem*> AffordableItems;

private:
	void OnIsAffordableResponse(bool bSuccess, TPair<URHStoreItem*, URHStoreItemPrice*> ItemPricePair);
};
