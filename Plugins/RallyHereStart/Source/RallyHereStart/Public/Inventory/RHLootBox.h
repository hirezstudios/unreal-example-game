// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Managers/RHStoreItemHelper.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHLootBox.generated.h"

UCLASS()
class RALLYHERESTART_API URHLootBox : public UPlatformInventoryItem
{
    GENERATED_BODY()

public:
    URHLootBox(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// This is the loot table item id that the loot box exchanges for in the Loot Box Redemption Vendor
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 LootBoxVendorId;

	virtual void CanOwnMore(const URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate) override;
};