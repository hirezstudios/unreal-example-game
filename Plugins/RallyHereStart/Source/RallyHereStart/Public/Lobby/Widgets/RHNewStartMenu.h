// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Engine/Texture2DDynamic.h"
#include "Managers/RHJsonDataFactory.h"
#include "RHNewStartMenu.generated.h"

UCLASS(BlueprintType)
class RALLYHERESTART_API URHNewStartMenuData : public URHJsonData
{
    GENERATED_BODY()

public:
    // Image reference if available
    UPROPERTY(BlueprintReadOnly)
    UTexture2DDynamic* Image;
};

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetIsNewsAvailableDynamicDelegate, bool, bAvailable);
DECLARE_DELEGATE_OneParam(FOnGetIsNewsAvailableDelegate, bool);
DECLARE_RH_DELEGATE_BLOCK(FOnGetIsNewsAvailableBlock, FOnGetIsNewsAvailableDelegate, FOnGetIsNewsAvailableDynamicDelegate, bool);

/**
 * New Start Menu Screen
 */
UCLASS()
class RALLYHERESTART_API URHNewStartMenuWidget : public URHWidget
{
    GENERATED_BODY()

public:
    virtual void InitializeWidget_Implementation() override;
    virtual void UninitializeWidget_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "New Start Main Widget", meta = (DisplayName = "Check Is News Available", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_CheckIsNewsAvailable(const FOnGetIsNewsAvailableDynamicDelegate& Delegate) { CheckIsNewsAvailable(Delegate); };
	void CheckIsNewsAvailable(FOnGetIsNewsAvailableBlock Delegate = FOnGetIsNewsAvailableBlock());

private:
    UFUNCTION(BlueprintPure)
    URHJsonDataFactory* GetJsonDataFactory();
};