// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "Shared/Settings/RHSettingsPreview.h"
#include "RHSettingsContainer.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsContainer : public URHWidget
{
	GENERATED_BODY()

public:
    URHSettingsContainer(const FObjectInitializer& ObjectInitializer);
    virtual void InitializeWidget_Implementation() override;
    virtual void SetIsEnabled(bool bInIsEnabled) override;

	bool OnSettingsStateChanged(const FRHSettingsState& SettingsState);

public:
    void SetContainerConfig(const class URHSettingsContainerConfigAsset* InContainerConfig);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Container")
    void OnContainerConfigSet();

public:
    UFUNCTION(BlueprintPure, Category = "Settings Container")
    FORCEINLINE FText GetWidgetContainerTitle() const { return ContainerConfigAsset->GetSettingName(); }

    UFUNCTION(BlueprintPure, Category = "Settings Container")
    FORCEINLINE FText GetWidgetContainerDescription() const { return ContainerConfigAsset->GetSettingDescription(); }

	UFUNCTION(BlueprintPure, Category = "Settings Container")
	FORCEINLINE class URHSettingsPreview* GetWidgetContainerPreview() const { return (AssociatePreviewWidget) ? AssociatePreviewWidget : nullptr; }

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Container")
    void AddSettingsWidget(class URHSettingsWidget* SettingsWidget);

    void ShowSettingsWidget(class URHSettingsWidget* SettingsWidget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Group")
    void OnShowSettingsWidget(class URHSettingsWidget* SettingsWidget);

    void HideSettingsWidget(class URHSettingsWidget* SettingsWidget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Group")
    void OnHideSettingsWidget(class URHSettingsWidget* SettingsWidget);

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Container")
	void AddPreviewWidget(class URHSettingsPreview* PreviewWidget);

public:
    UFUNCTION(BlueprintPure, Category = "Settings Container")
    FORCEINLINE TArray<class URHSettingsWidget*>& GetSettingsWidgets() { return SettingsWidgets; }
protected:
	UPROPERTY(Transient)
    TArray<class URHSettingsWidget*> SettingsWidgets;

	UPROPERTY(Transient)
	class URHSettingsPreview* AssociatePreviewWidget;

protected:
    UPROPERTY(BlueprintReadOnly)
    const class URHSettingsContainerConfigAsset* ContainerConfigAsset;
};
