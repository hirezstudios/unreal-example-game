// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHOrderManager.h"
#include "Managers/RHViewManager.h"
#include "RHOrderModal.generated.h"

USTRUCT(BlueprintType)
struct FOrderHeaderOverrides : public FTableRowBase
{
	GENERATED_BODY()

	// The Header displayed for the override
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Header;

	// The header displays if any of the given items in the order are in this list
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRH_LootId> LootTableItemIds;
};

UCLASS(Blueprintable)
class RALLYHERESTART_API URHOrderViewRedirector : public URHViewRedirecter
{
	GENERATED_BODY()

public:
	// Types of Orders that this should capture to redirect to
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<ERHOrderType> ValidOrderTypes;


	virtual bool ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData); //$$ KAB - Route names changed to Gameplay Tags
};

/**
 * Order modal widget
 */
UCLASS()
class RALLYHERESTART_API URHOrderModal : public URHWidget
{
	GENERATED_BODY()

public:
	// Initializes the base setup for the widget
	virtual void InitializeWidget_Implementation() override;

protected:
	// get the order manager
	UFUNCTION(BlueprintPure)
	URHOrderManager*	GetOrderManager() const;

	// Returns the header text for the order
	UFUNCTION(BlueprintPure)
	FText					GetHeaderText(const URHOrder* Order) const;

	// Table that contains header overrides for the order window
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AssetBundles = "Data"))
	UDataTable* HeaderOverridesTable;

	// A list of header overrives populated externally through json
	UPROPERTY(Transient)
	TMap<FRH_LootId, FText> HeaderOverridesFromJson;
};