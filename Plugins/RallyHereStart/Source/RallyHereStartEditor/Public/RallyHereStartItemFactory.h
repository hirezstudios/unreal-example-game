// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RallyHereStartItemFactory.generated.h"

/**
 *
 */

UCLASS(MinimalAPI, HideCategories=Object)
class URallyHereStartItemFactory : public UFactory
{
    GENERATED_BODY()

public:
	URallyHereStartItemFactory(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    UPROPERTY(EditAnywhere, Category = "Item")
    TSubclassOf<UPlatformInventoryItem> RHItemClass;

    // UFactory interface
    virtual bool ConfigureProperties() override;
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
    // End of UFactory interface
};