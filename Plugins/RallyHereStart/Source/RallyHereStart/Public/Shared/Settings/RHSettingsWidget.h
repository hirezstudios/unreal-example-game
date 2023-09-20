// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "Shared/Settings/RHSettingsPreview.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsWidget.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsWidget : public URHWidget
{
	GENERATED_BODY()

public:
    URHSettingsWidget(const FObjectInitializer& ObjectInitializer);

	void InitializeFromWidgetConfig(class ARHHUDCommon* HUD, const struct FRHSettingsWidgetConfig& InWidgetConfig);

    virtual void InitializeWidget_Implementation() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget")
    void OnInputAttached(bool bGamepadAttached, bool bMouseAttached);

	UFUNCTION(BlueprintPure, Category = "Settings Widget")
	FORCEINLINE bool HasPreview() const { return bHasPreview; }
    
public:
    void SetWidgetConfig(const struct FRHSettingsWidgetConfig& InWidgetConfig);
    void SetWidgetContainerTitle(const FText& InWidgetContainerTitle);
    void SetWidgetContainerDescription(const FText& InWidgetContainerDescription);
	void SetWidgetContainerPreviewWidget(class URHSettingsPreview*& InWidgetContainerPreviewWidget);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget")
    void OnWidgetConfigSet();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget")
	void OnSettingsInfoValueChanged(bool bChangedExternally);

    UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Settings Widget")
    bool CanGamepadNavigate();

public:
    void SetWidgetSettingsInfo(class URHSettingsInfoBase* InSettingsInfo);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget")
    void OnWidgetSettingsInfoSet();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget")
    void OnWidgetContainerTitleSet();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget")
    void OnWidgetContainerDescriptionSet();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget")
	void OnWidgetContainerPreviewSet();

protected:
    UPROPERTY(BlueprintReadOnly)
    struct FRHSettingsWidgetConfig WidgetConfig;

    UPROPERTY(BlueprintReadOnly)
    FText WidgetContainerTitle;

    UPROPERTY(BlueprintReadOnly)
    FText WidgetContainerDescription;

	UPROPERTY(BlueprintReadOnly)
	bool bHasPreview;

	UPROPERTY(BlueprintReadOnly)
	class URHSettingsPreview* WidgetContainerPreviewWidget;

    UPROPERTY(BlueprintReadOnly, Category = "Settings Widget")
    class URHSettingsInfoBase* SettingsInfo;

public:
    UFUNCTION(BlueprintCallable, Category = "Settings Widget")
    void ApplySetting();

    UFUNCTION(BlueprintPure, Category = "Settings Widget")
    bool IsApplied();

    UFUNCTION(BlueprintCallable, Category = "Settings Widget")
    void SaveSetting();

    UFUNCTION(BlueprintPure, Category = "Settings Widget")
    bool IsSaved();

    UFUNCTION(BlueprintCallable, Category = "Settings Widget")
    void RevertSetting();
};
