// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "GameFramework/RHGameInstance.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RH_PlayerInventory.h"
#include "RHActivity.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogActivity, Log, All);

class URHActivityInstance;

UCLASS()
class RALLYHERESTART_API URHActivity : public UPlatformInventoryItem
{
	GENERATED_BODY()
	
public:
    URHActivity(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Checks if the activity is active based on a variety of rules for the given activity type
	void IsActive(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());

	// Gets the progress the player has into the activity
	void GetCurrentProgress(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryCountBlock& Delegate = FRH_GetInventoryCountBlock());

	// Gets the total amount needed to complete the activity
	int32 GetCompletionProgressCount();

	// Checks if the player has gotten to the goal amount of the activity
	void HasBeenCompleted(URH_PlayerInfo* PlayerInfo, const FRH_GetInventoryStateBlock& Delegate = FRH_GetInventoryStateBlock());

protected:
	// If set, then the player must own this item for this activity to be active
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activity")
	FRH_ItemId RequiredItemId;

	// Loot Id that the instance server uses to grant progress to the player for the activity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activity")
	FRH_LootId ProgressLootId;

	// Used by server when creating the activity instance for this activity to track its progress
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activity")
	TSoftClassPtr<URHActivityInstance> ActivityInstanceClass;

	// This will call into the activity and use the profile to add loot rewards to be given to the player
	bool AddLootReward(URHGameInstance* GameInstance, class URH_PlayerInfo* PlayerInfo, int32 Count) const;
};
