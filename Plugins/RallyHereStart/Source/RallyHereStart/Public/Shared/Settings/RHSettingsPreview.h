// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsPreview.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreviewValueChanged);

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsPreview : public URHWidget
{
	GENERATED_BODY()

public:
	URHSettingsPreview(const FObjectInitializer& ObjectInitializer);

    virtual void InitializeWidget_Implementation() override;

	void SetWidgetSettingsInfo(class URHSettingsInfoBase* InSettingsInfo);

	UPROPERTY(BlueprintAssignable, Category = "Setting Preview")
	FOnPreviewValueChanged OnPreviewValueChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Settings Widget")
	class URHSettingsInfoBase* SettingsInfo;

private:
	UFUNCTION()
	void HandleOnValueChanged(bool ChangedExternally);
	UFUNCTION()
	void HandleOnPreviewValueChanged();
};
