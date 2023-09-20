// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RHAnalogStickFilter.h"
#include "RHCircleDeadZoneFilter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class RALLYHERESTART_API URHCircleDeadZoneFilter : public URHAnalogStickFilter
{
	GENERATED_BODY()
	
public:
    URHCircleDeadZoneFilter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual FVector2D FilterStickInput(FVector2D RawStickInput) override;

protected:
    //Input is in the [-1,1] range.
    UPROPERTY(Config)
    float DeadZoneRadius;

    UPROPERTY(Config)
    float CardinalDeadZoneHalfWidth;
};
