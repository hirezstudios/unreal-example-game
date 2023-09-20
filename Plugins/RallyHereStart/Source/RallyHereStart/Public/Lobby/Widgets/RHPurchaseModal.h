// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHStoreItemHelper.h"
#include "RHPurchaseModal.generated.h"

/**
 * Purchase modal widget
 */
UCLASS()
class RALLYHERESTART_API URHPurchaseModal : public URHWidget
{
    GENERATED_BODY()

    // initialization and bootstrapping
public:
    UFUNCTION(BlueprintCallable)
    void                                             SetupBindings();

    // basic internal utility
protected:
    // get the store item helper
    UFUNCTION(BlueprintCallable)
    URHStoreItemHelper*                           GetStoreItemHelper();

    // specialized functions
public:
    UFUNCTION(BlueprintNativeEvent)
    void                                             HandleShowPurchaseModal(URHStoreItem* Item, URHStoreItemPrice* Price);
    
    // internal properties
protected:

    // exposed properties
private:
};
