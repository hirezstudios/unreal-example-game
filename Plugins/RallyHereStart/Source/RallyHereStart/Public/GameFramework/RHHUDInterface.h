// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RHSettingsCallbackInterface.h"
#include "RHHUDInterface.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FRHHUDMessageDelegate, FName, Message);

USTRUCT(BlueprintType)
struct RALLYHERESTART_API FRHWidgetInfoParams
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    TSoftClassPtr<UUserWidget> Widget;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    bool bPreloadWidget;

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FString WidgetParentTarget;

    UPROPERTY(BlueprintReadOnly)
    class AActor* InfoActor;

	FRHWidgetInfoParams()
        : bPreloadWidget(false)
        , InfoActor(nullptr)
    {
    }
};

UINTERFACE(BlueprintType)
class RALLYHERESTART_API URHHUDInterface : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class RALLYHERESTART_API IRHHUDInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GM Commands | ShowHUD")
    void OnToggleHUD();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GM Commands | ShowHUD")
    void SetHUDVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RH HUD Interface | Safe Frame")
	void SetSafeFrameScale(float SafeFrameScale);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RH HUD Interface | Game Rule Widget")
    void CreateGameRuleWidget(FRHWidgetInfoParams WidgetInfoParams);
	
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "RH HUD Interface | Widget Messaging")
    void BroadcastWidgetMessage(FName Message);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "RH HUD Interface | Widget Messaging")
    void BindEventToWidgetMessages(const FRHHUDMessageDelegate& Callback);

    UFUNCTION()
    virtual void OnLoadRoute(FName Route, bool ForceTransition) PURE_VIRTUAL(IRHHUDInterface::OnLoadRoute, return;);

	UFUNCTION(BlueprintNativeEvent, Category = "RH HUD Interface | Game Mode")
	void ReceivedGameModeClass(TSubclassOf<AGameModeBase> GameModeClass);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Settings")
	TScriptInterface<IRHSettingsCallbackInterface> GetSettingsCallbackInterface() const;
	virtual TScriptInterface<IRHSettingsCallbackInterface> GetSettingsCallbackInterface_Implementation() const PURE_VIRTUAL(IRHHUDInterface::GetSettingsCallbackInterface_Implementation, return TScriptInterface<IRHSettingsCallbackInterface>();)
};