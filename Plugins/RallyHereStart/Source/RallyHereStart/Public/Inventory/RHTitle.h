// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHTitle.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHTitle : public UPlatformInventoryItem
{
    GENERATED_BODY()

public:
    URHTitle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Title")
    FText TitleText;
};