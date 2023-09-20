// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RHAnalogStickFilter.h"
#include "RHBoxDeadZoneFilter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class RALLYHERESTART_API URHBoxDeadZoneFilter : public URHAnalogStickFilter
{
	GENERATED_BODY()

public:
	URHBoxDeadZoneFilter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FVector2D FilterStickInput(FVector2D RawStickInput);

	UPROPERTY(Config, EditDefaultsOnly, Category="Dead Zone")
	FVector2D InnerDeadZone;
	UPROPERTY(Config, EditDefaultsOnly, Category = "Dead Zone")
	FVector2D OuterDeadZone;
};
