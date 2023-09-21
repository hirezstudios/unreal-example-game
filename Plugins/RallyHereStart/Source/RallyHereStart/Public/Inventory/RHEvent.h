// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHEvent.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHEvent : public UPlatformInventoryItem
{
	GENERATED_BODY()
	
public:
    URHEvent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Gets the remaining time until the event is no longer active
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	int32 GetRemainingSeconds(const UObject* WorldContextObject) const;

	virtual void AppendRequiredLootTableIds(TArray<int32>& LootTableIds) { }

	// The Tag associated with the event, when loaded is set by the EventManager
	FName EventTag;
};
