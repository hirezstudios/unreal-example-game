// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHProgression.generated.h"

UCLASS()
class RALLYHERESTART_API URHProgression : public UPlatformInventoryItem
{
    GENERATED_BODY()

public:
	URHProgression(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly)
	int32 ProgressionXpTableId;

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	int32 GetProgressionLevel(const class UObject* WorldContextObject, int32 ProgressAmount) const;

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	float GetProgressionLevelPercent(const class UObject* WorldContextObject, int32 ProgressAmount) const;
};