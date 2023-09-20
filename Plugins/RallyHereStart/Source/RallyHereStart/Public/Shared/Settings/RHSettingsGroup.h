// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "RHSettingsGroup.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsGroup : public URHWidget
{
	GENERATED_BODY()

public:
    URHSettingsGroup(const FObjectInitializer& ObjectInitializer);
    virtual void InitializeWidget_Implementation() override;

	bool OnSettingsStateChanged(const FRHSettingsState& SettingsState);

    void SetGroupConfig(const struct FRHSettingsGroupConfig& InGroupConfig);

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Group")
    void OnGroupConfigSet();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Group")
    void AddMainSettingsContainerWidget(class URHSettingsContainer* SettingsContainer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Group")
    void AddSubSettingsContainerWidget(class URHSettingsContainer* SettingsContainer);

    void ShowContainer(class URHSettingsContainer* SettingsContainer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Group")
    void OnShowContainer(class URHSettingsContainer* SettingsContainer);

    void HideContainer(class URHSettingsContainer* SettingsContainer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Settings Group")
    void OnHideContainer(class URHSettingsContainer* SettingsContainer);

public:
    UFUNCTION(BlueprintPure, Category = "Settings Group")
    FORCEINLINE TArray<class URHSettingsContainer*>& GetSettingsContainers() { return SettingsContainers; }
protected:
	UPROPERTY(Transient)
    TArray<class URHSettingsContainer*> SettingsContainers;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Settings Group")
    TSubclassOf<class URHSettingsContainer> SettingsContainerClass;

protected:
    UPROPERTY(BlueprintReadOnly)
    struct FRHSettingsGroupConfig GroupConfig;

public:
    static bool IsContainerConfigAllowed(const class URHSettingsContainerConfigAsset* ContainerConfig);
};
