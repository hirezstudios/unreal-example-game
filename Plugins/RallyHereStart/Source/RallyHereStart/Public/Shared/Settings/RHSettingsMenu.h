// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "RHSettingsMenu.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsMenu : public URHWidget
{
	GENERATED_BODY()

public:
    URHSettingsMenu(const FObjectInitializer& ObjectInitializer);
    virtual void InitializeWidget_Implementation() override;

	virtual void ShowWidget(ESlateVisibility InVisibility = ESlateVisibility::SelfHitTestInvisible) override; //$$ LDP - Added visibility param

protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Settings Menu")
    void RebuildNavigation();

private:
	UPROPERTY(transient)
	FRHSettingsState SettingsState;

	void OnSettingsStateChanged();

public:
    void SetMenuConfig(const class URHSettingsMenuConfigAsset* InMenuConfigAsset);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Menu")
    void OnMenuConfigSet();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Menu")
    void AddSettingsPageWidget(class URHSettingsPage* SettingsPage);

    void ShowPage(class URHSettingsPage* SettingsPage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Menu")
    void OnShowPage(class URHSettingsPage* SettingsPage);

    void HidePage(class URHSettingsPage* SettingsPage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Menu")
    void OnHidePage(class URHSettingsPage* SettingsPage);

    // Called to pop popup to get waht to do with pending changes on exiting screen
    UFUNCTION(BlueprintCallable)
    void CheckSavePendingChanges();

    // Callback when attempting to close screen to save or dump pending changes
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Menu")
    void OnConfirmExit(bool ShouldSaveSettings);

	// RevertSettings to be implemented in WBP_SettingsMenu
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Settings Menu")
	void RevertSettings();

	// Call popup to confirm reverting settings
	UFUNCTION(BlueprintCallable)
	void ConfirmRevertSettings();

    // User has selected to revert settings on exit
    UFUNCTION()
    void OnRevertSettings();

    // User has selected to save settings on exit
    UFUNCTION()
    void OnSaveSettings();

public:
    UFUNCTION(BlueprintPure, Category = "Settings Menu")
    FORCEINLINE TArray<class URHSettingsPage*>& GetSettingsPages() { return SettingsPages; }
protected:
	UPROPERTY(Transient)
    TArray<class URHSettingsPage*> SettingsPages;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Settings Menu")
    TSubclassOf<class URHSettingsPage> SettingsPageClass;

protected:
	UPROPERTY(BlueprintReadOnly)
    const class URHSettingsMenuConfigAsset* MenuConfigAsset;

public:
    static bool IsPageConfigAllowed(const class URHSettingsPageConfigAsset* PageConfigAsset);
};
