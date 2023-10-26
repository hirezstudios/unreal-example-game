#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHCurrencyDisplay.generated.h"

UCLASS()
class RALLYHERESTART_API URHCurrencyDisplay : public URHWidget
{
    GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetCurrentDisplayByItem(URHStoreItem* StoreItem);

	UFUNCTION(BlueprintCallable)
	void SetInventoryCountOnText(const FRH_ItemId& ItemId, UTextBlock* TextBlock);
};