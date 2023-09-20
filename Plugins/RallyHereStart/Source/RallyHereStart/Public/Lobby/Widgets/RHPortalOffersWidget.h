#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHStoreItemHelper.h"
#include "RHPortalOffersWidget.generated.h"

UCLASS()
class RALLYHERESTART_API URHPortalOffersWidget : public URHWidget
{
    GENERATED_BODY()

protected:
    UFUNCTION(BlueprintPure)
    URHStoreItemHelper*         GetItemHelper() const;

    UFUNCTION(BlueprintPure)
    TArray<URHStoreItem*>    GetPortalOfferItems() const;
};
