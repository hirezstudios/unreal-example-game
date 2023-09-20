// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "RHSettingsPage.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsPage : public URHWidget
{
	GENERATED_BODY()

public:
    URHSettingsPage(const FObjectInitializer& ObjectInitializer);
    virtual void InitializeWidget_Implementation() override;

	bool OnSettingsStateChanged(const FRHSettingsState& SettingsState);

    void SetPageConfig(const class URHSettingsPageConfigAsset* InPageConfigAsset);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Page")
    void OnPageConfigSet();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Page")
    void AddSettingsSectionWidget(class URHSettingsSection* SettingsSection);

    void ShowSection(class URHSettingsSection* SettingsSection);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Page")
    void OnShowSection(class URHSettingsSection* SettingsSection);

    void HideSection(class URHSettingsSection* SettingsSection);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Page")
    void OnHideSection(class URHSettingsSection* SettingsSection);

public:
    UFUNCTION(BlueprintPure, Category = "Settings Page")
    FORCEINLINE TArray<class URHSettingsSection*>& GetSettingsSections() { return SettingsSections; }
protected:
	UPROPERTY(Transient)
    TArray<class URHSettingsSection*> SettingsSections;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Settings Page")
    TSubclassOf<class URHSettingsSection> SettingsSectionClass;

public:
    UFUNCTION(BlueprintImplementableEvent, BlueprintPure, Category = "Settings Page")
    UScrollBox* GetScrollBox();

protected:
	UPROPERTY(BlueprintReadOnly)
	const class URHSettingsPageConfigAsset* PageConfigAsset;

public:
	static bool IsSectionConfigAllowed(const URHSettingsSectionConfigAsset* SectionConfig);
};
