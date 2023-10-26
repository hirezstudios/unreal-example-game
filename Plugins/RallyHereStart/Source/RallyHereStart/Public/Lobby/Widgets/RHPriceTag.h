#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHPriceTag.generated.h"

UCLASS()
class RALLYHERESTART_API URHPriceTag : public URHWidget
{
    GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetPriceTag(URHStoreItemPrice* StoreItemPrice, URHStoreItem* StoreItem);
};