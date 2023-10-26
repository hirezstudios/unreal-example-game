// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
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

    // specialized functions
public:
    UFUNCTION(BlueprintNativeEvent)
    void                                             HandleShowPurchaseModal(URHStoreItem* Item, URHStoreItemPrice* Price);
    
    // internal properties
protected:

    // exposed properties
private:
};
