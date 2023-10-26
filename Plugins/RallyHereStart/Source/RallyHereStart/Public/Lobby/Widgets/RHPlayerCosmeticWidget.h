#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHPlayerCosmeticWidget.generated.h"

DECLARE_DELEGATE(FRH_OnOwnershipCheckCompleteDelegate);

USTRUCT(BlueprintType)
struct FRHProfileItemsWrapper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<UPlatformInventoryItem*> Items;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetCosmeticItems, FRHProfileItemsWrapper, ItemWrapper);

UCLASS()
class URH_PlayerCosmeticOwnershipHelper : public UObject
{
	GENERATED_BODY()

public:
	void StartCheck();

	FRH_OnOwnershipCheckCompleteDelegate OnCompleteDelegate;
	int32 Results;
	bool IsComplete;

	FOnGetCosmeticItems Event;
	TArray<UPlatformInventoryItem*> ItemsToCheck;
	FRHProfileItemsWrapper ResultList;

	UPROPERTY()
	const URH_PlayerInfo* PlayerInfo;

private:
	void OnIsOwnedResponse(bool bSuccess, UPlatformInventoryItem* Item);
};

UCLASS()
class RALLYHERESTART_API URHPlayerCosmeticWidget : public URHWidget
{
	GENERATED_BODY()

protected:
	// Gets all of the Cosmetic Items that that the player owns as Store Items
	UFUNCTION(BlueprintCallable)
	bool GetItemsForSlot(FOnGetCosmeticItems Event, ERHLoadoutSlotTypes SlotType);

	bool GetItemsByTag(FOnGetCosmeticItems Event, ERHLoadoutSlotTypes SlotType, FName TagName);

	void AdditionalAssetsAsyncLoaded(FOnGetCosmeticItems Event, ERHLoadoutSlotTypes SlotType, TArray<TSoftObjectPtr<UPlatformInventoryItem>> Items);

protected:
	UPROPERTY(Transient)
	TArray<URH_PlayerCosmeticOwnershipHelper*> PendingOwnershipRequests;
};