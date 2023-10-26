#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHViewItemsWidget.generated.h"

class URHStoreItem;

UCLASS(BlueprintType)
class RALLYHERESTART_API URHViewItemData : public URHWidget
{
	GENERATED_BODY()

public:
	// Store Item to display on screen
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	TArray<TSoftObjectPtr<URHStoreItem>> StoreItems;

	// Platform Inventory Item to Display on screen if no StoreItem available
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	UPlatformInventoryItem* InventoryItem;

	// If set override the default camera
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FName SceneCamera;

	// If set override the default view model
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FName SceneViewModel;
};

UCLASS()
class RALLYHERESTART_API URHViewItemsWidget : public URHWidget
{
    GENERATED_BODY()
};