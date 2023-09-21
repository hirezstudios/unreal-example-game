// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RHGamepadLookSpeedManager.generated.h"

class ARHPlayerController;

/**
 * 
 */
UCLASS(BlueprintType, Abstract, Config=Input)
class RALLYHERESTART_API URHGamepadLookSpeedManager : public UObject
{
	GENERATED_BODY()
	
public:
	URHGamepadLookSpeedManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Init();
	virtual FVector2D UpdateGamepadLook(FVector2D GamepadInput, float DeltaTime);
	FORCEINLINE ARHPlayerController* GetPlayerController() const { return PlayerController; }

protected:
	UPROPERTY(Transient, DuplicateTransient)
	ARHPlayerController* PlayerController;
};
