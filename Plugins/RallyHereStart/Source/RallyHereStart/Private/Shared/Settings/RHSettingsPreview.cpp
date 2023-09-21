// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/RHSettingsPreview.h"

URHSettingsPreview::URHSettingsPreview(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void URHSettingsPreview::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

void URHSettingsPreview::SetWidgetSettingsInfo(class URHSettingsInfoBase* InSettingsInfo)
{
	if (InSettingsInfo != nullptr)
	{
		SettingsInfo = InSettingsInfo;

		SettingsInfo->OnSettingValueChanged.AddDynamic(this, &URHSettingsPreview::HandleOnValueChanged);
		SettingsInfo->OnSettingPreviewChanged.AddDynamic(this, &URHSettingsPreview::HandleOnPreviewValueChanged);
	}
}

void URHSettingsPreview::HandleOnValueChanged(bool ChangedExternally)
{
	OnPreviewValueChanged.Broadcast();
}

void URHSettingsPreview::HandleOnPreviewValueChanged()
{
	OnPreviewValueChanged.Broadcast();
}
