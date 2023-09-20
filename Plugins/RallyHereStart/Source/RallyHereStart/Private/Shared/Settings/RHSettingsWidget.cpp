// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/RHSettingsWidget.h"

URHSettingsWidget::URHSettingsWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
	, bHasPreview(false)
    , SettingsInfo(nullptr)
{

}

void URHSettingsWidget::InitializeFromWidgetConfig(class ARHHUDCommon* HUD, const struct FRHSettingsWidgetConfig& InWidgetConfig)
{
	if (HUD != nullptr)
	{
		InitializeWidget();
		SetWidgetConfig(InWidgetConfig);

		if (URHSettingsInfoBase* pSettingsInfo = URHSettingsInfoBase::CreateRHSettingsInfo(this, WidgetConfig.SettingInfo))
		{
			SetWidgetSettingsInfo(pSettingsInfo);
		}
	}
}

void URHSettingsWidget::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

void URHSettingsWidget::SetWidgetConfig(const struct FRHSettingsWidgetConfig& InWidgetConfig)
{
    WidgetConfig = InWidgetConfig;

    OnWidgetConfigSet();
}

void URHSettingsWidget::SetWidgetContainerTitle(const FText& InWidgetContainerTitle)
{
    WidgetContainerTitle = InWidgetContainerTitle;

    OnWidgetContainerTitleSet();
}

void URHSettingsWidget::SetWidgetContainerDescription(const FText& InWidgetContainerDescription)
{
    WidgetContainerDescription = InWidgetContainerDescription;

    OnWidgetContainerDescriptionSet();
}

void URHSettingsWidget::SetWidgetContainerPreviewWidget(class URHSettingsPreview*& InWidgetContainerPreviewWidget)
{
	WidgetContainerPreviewWidget = InWidgetContainerPreviewWidget;
	bHasPreview = true;

	if(SettingsInfo != nullptr)
	{
		WidgetContainerPreviewWidget->SetWidgetSettingsInfo(SettingsInfo);
	}

	OnWidgetContainerPreviewSet();
}

bool URHSettingsWidget::CanGamepadNavigate_Implementation()
{
    switch (GetVisibility())
    {
    case ESlateVisibility::Collapsed:
    case ESlateVisibility::Hidden:
        return false;
    default:
        return GetIsEnabled();
    }
}

void URHSettingsWidget::SetWidgetSettingsInfo(class URHSettingsInfoBase* InSettingsInfo)
{
    SettingsInfo = InSettingsInfo;

    OnWidgetSettingsInfoSet();

	if (SettingsInfo != nullptr)
	{
		SettingsInfo->OnSettingValueChanged.AddDynamic(this, &URHSettingsWidget::OnSettingsInfoValueChanged);
	}

	OnSettingsInfoValueChanged(true);
}

void URHSettingsWidget::ApplySetting()
{
    if (URHSettingsInfoBase* const pSettingsInfo = SettingsInfo)
    {
        pSettingsInfo->Apply();
    }
}

bool URHSettingsWidget::IsApplied()
{
    if (URHSettingsInfoBase* const pSettingsInfo = SettingsInfo)
    {
        return !pSettingsInfo->IsDirty();
    }
    return false;
}

void URHSettingsWidget::SaveSetting()
{
    if (URHSettingsInfoBase* const pSettingsInfo = SettingsInfo)
    {
        pSettingsInfo->Save();
    }
}

bool URHSettingsWidget::IsSaved()
{
    if (URHSettingsInfoBase* const pSettingsInfo = SettingsInfo)
    {
        return !pSettingsInfo->CanRevert();
    }
    return false;
}

void URHSettingsWidget::RevertSetting()
{
    if (URHSettingsInfoBase* const pSettingsInfo = SettingsInfo)
    {
        pSettingsInfo->Revert();
    }
}
