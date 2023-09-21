// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHStoreItemHelper.h"
#include "Managers/RHStoreItemHelper.h"
#include "RHStoreWidget.generated.h"

// Enum to tell the UI which type of store widget panel to create for the given item.
UENUM(BlueprintType)
enum EStoreItemWidgetType
{
    ELargePanel,
    ETallPanel,
    ESmallPanel,
	EWidePanel,
    STORE_WIDGET_TYPE_MAX
};

// An Individual item on a store panel
UCLASS(BlueprintType)
class RALLYHERESTART_API URHStorePanelItem : public UObject
{
	GENERATED_BODY()

public:
	// The Item to be displayed on the panel
	UPROPERTY(BlueprintReadOnly)
	URHStoreItem* StoreItem;

	UPROPERTY(BlueprintReadOnly)
	bool IsNew;

	UPROPERTY()
	bool DisplaySaleTag;

	UPROPERTY(BlueprintReadOnly)
	bool HasBeenSeen;

	UPROPERTY(BlueprintReadOnly)
	FText CustomBannerText;

	UFUNCTION(BlueprintPure)
	bool IsOnSale() const { return DisplaySaleTag || StoreItem->IsOnSale(); }
};

// An individual panel of items in the store
UCLASS(BlueprintType)
class RALLYHERESTART_API URHStoreSectionItem : public UObject
{
    GENERATED_BODY()

public:
	// All of the items that are displayed on the given panel
	UPROPERTY(BlueprintReadWrite)
	TArray<URHStorePanelItem*> StorePanelItems;

    // The column the panel appears in its section
    UPROPERTY(BlueprintReadOnly)
    int32 Column;

    // The row the panel appears in its section
    UPROPERTY(BlueprintReadOnly)
    int32 Row;

    // The Items widget type (size) that is placed in the layout
    UPROPERTY(BlueprintReadOnly)
    TEnumAsByte<EStoreItemWidgetType> WidgetType;

    // This is used by the blueprint when drilling into a store item to display the current panel index on the Purchase Confirmation first
    UPROPERTY(BlueprintReadWrite)
	URHStorePanelItem* CurrentlyViewedItem;

    // Gets if the panel has an unseen item on it
	UFUNCTION(BlueprintPure)
	bool HasUnseenItems() const;
};

// A collection of panels under a given section in the store
UCLASS(BlueprintType)
class RALLYHERESTART_API URHStoreSection : public UObject
{
    GENERATED_BODY()

public:
    // All panels that are displayed in the given section
    UPROPERTY(BlueprintReadOnly)
    TArray<URHStoreSectionItem*> SectionItems;

    // Returns the section header text based on the UIHintId
    UFUNCTION(BlueprintPure)
    FText GetSectionHeader() const;

    // Internal Section Type shared by all items on this panel section
    UPROPERTY(BlueprintReadOnly)
    TEnumAsByte<EStoreSectionTypes> SectionType;

	FDateTime SectionRotationEndTime;

	// This is a custom header injected by JSON pulled based on localization of client
	FText CustomHeader;

	// Gets if the section has unseen items
	UFUNCTION(BlueprintPure)
	bool HasUnseenItems() const;

    UFUNCTION(BlueprintPure)
    int32 GetSecondsRemaining() const;
};

UCLASS(Config=Game)
class RALLYHERESTART_API URHStoreWidget : public URHWidget
{
    GENERATED_BODY()

public:
    // Initializes the base setup for the widget
    virtual void InitializeWidget_Implementation() override;

    // Cleans up the base setup for the widget
    virtual void UninitializeWidget_Implementation() override;

    // Returns all of the store data sorted by sections ready to be displayed
    UFUNCTION(BlueprintCallable)
    TArray<URHStoreSection*> GetStoreLayout(int32& ErrorCode);

    // Callback for when we get Portal Offers
    UFUNCTION(BlueprintImplementableEvent)
    void OnPortalOffersReceived();

	// Callback when price points are received
	UFUNCTION(BlueprintImplementableEvent)
	void OnPricePointsReveived();

	UFUNCTION(BlueprintPure)
	bool HasAllRequiredStoreInformation() const;

protected:
    // Returns the StoreItemHelper
    UFUNCTION(BlueprintPure)
    URHStoreItemHelper* GetStoreItemHelper() const;

	// Parses items into panels for the section
	URHStoreSection* CreateStoreSection(TArray<URHStoreItem*>& StoreItems, EStoreSectionTypes SectionType, EStoreItemWidgetType PanelSize, bool SinglePanel = false);
};