#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHPortalOffersWidget.generated.h"

UCLASS()
class RALLYHERESTART_API URHPortalOffersWidget : public URHWidget
{
    GENERATED_BODY()

protected:
    UFUNCTION(BlueprintPure)
    TArray<URHStoreItem*>    GetPortalOfferItems() const;
};
