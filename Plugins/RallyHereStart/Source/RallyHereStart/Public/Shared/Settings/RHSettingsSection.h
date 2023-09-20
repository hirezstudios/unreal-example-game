// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "RHSettingsSection.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsSection : public URHWidget
{
	GENERATED_BODY()

public:
    URHSettingsSection(const FObjectInitializer& ObjectInitializer);
    virtual void InitializeWidget_Implementation() override;

	bool OnSettingsStateChanged(const FRHSettingsState& SettingsState);

    void SetSectionConfig(const class URHSettingsSectionConfigAsset* InSectionConfigAsset);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Section")
    void OnSectionConfigSet();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Section")
    void AddSettingsGroupWidget(class URHSettingsGroup* SettingsGroup);

    void ShowGroup(class URHSettingsGroup* SettingsGroup);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Section")
    void OnShowGroup(class URHSettingsGroup* SettingsGroup);

    void HideGroup(class URHSettingsGroup* SettingsGroup);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Section")
    void OnHideGroup(class URHSettingsGroup* SettingsGroup);

public:
    UFUNCTION(BlueprintPure, Category = "Settings Section")
    FORCEINLINE TArray<class URHSettingsGroup*>& GetSettingsGroups() { return SettingsGroups; }
protected:
	UPROPERTY(Transient)
    TArray<class URHSettingsGroup*> SettingsGroups;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Settings Section")
    TSubclassOf<class URHSettingsGroup> SettingsGroupClass;

protected:
	UPROPERTY(BlueprintReadOnly)
	const class URHSettingsSectionConfigAsset* SectionConfigAsset;

public:
    static bool IsGroupConfigAllowed(const FRHSettingsGroupConfig& GroupConfig);
};
