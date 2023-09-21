// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RHAnalogStickFilter.generated.h"

UENUM(BlueprintType)
enum class EAnalogStickType : uint8
{
	Unknown,
	Left,
	Right,
};

/**
 * 
 */
UCLASS(BlueprintType, Config=Input, Abstract)
class RALLYHERESTART_API URHAnalogStickFilter : public UObject
{
	GENERATED_BODY()

public:
	URHAnalogStickFilter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FVector2D FilterStickInput(FVector2D RawStickInput);

	UPROPERTY(Transient)
	EAnalogStickType StickType;

	UPROPERTY(Transient)
	class URHPlayerInput* PlayerInput;
};
