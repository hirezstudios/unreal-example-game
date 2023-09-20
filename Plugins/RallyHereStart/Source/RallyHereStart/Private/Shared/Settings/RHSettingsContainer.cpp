// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/RHSettingsContainer.h"
#include "Shared/Settings/RHSettingsWidget.h"
#include "Shared/Settings/RHSettingsPreview.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RHUIBlueprintFunctionLibrary.h"

URHSettingsContainer::URHSettingsContainer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URHSettingsContainer::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

void URHSettingsContainer::SetIsEnabled(bool bInIsEnabled)
{
    for (auto& SettingsWidget : SettingsWidgets)
    {
        SettingsWidget->SetIsEnabled(bInIsEnabled);
    }
    Super::SetIsEnabled(bInIsEnabled);
}

bool URHSettingsContainer::OnSettingsStateChanged(const FRHSettingsState& SettingsState)
{
	const bool bHasRequiredInput = (!ContainerConfigAsset->RequiredInputTypes.Gamepad || SettingsState.bIsGamepadAttached) && 
		(!ContainerConfigAsset->RequiredInputTypes.Mouse || SettingsState.bIsMouseAttached) &&
		(!ContainerConfigAsset->RequiredInputTypes.Touch || SettingsState.bIsTouchMode);

	if (bHasRequiredInput)
	{
		for (const auto& SettingsWidget : SettingsWidgets)
		{
			SettingsWidget->OnInputAttached(SettingsState.bIsGamepadAttached, SettingsState.bIsMouseAttached);
			ShowSettingsWidget(SettingsWidget);
		}
		return true;
	}
	else
	{
		for (const auto& SettingsWidget : SettingsWidgets)
		{
			SettingsWidget->OnInputAttached(SettingsState.bIsGamepadAttached, SettingsState.bIsMouseAttached);
			HideSettingsWidget(SettingsWidget);
		}
		return false;
	}
}

void URHSettingsContainer::SetContainerConfig(const class URHSettingsContainerConfigAsset* InContainerConfigAsset)
{
    ContainerConfigAsset = InContainerConfigAsset;

    if (MyHud.IsValid())
    {
        for (const FRHSettingsWidgetConfig& WidgetConfig : ContainerConfigAsset->WidgetConfigs)
        {
            if (class URHSettingsWidget* const SettingsWidget = URHUIBlueprintFunctionLibrary::CreateSettingsWidget(MyHud.Get(), WidgetConfig.WidgetClass))
            {
				SettingsWidget->InitializeFromWidgetConfig(MyHud.Get(), WidgetConfig);

				SettingsWidget->SetWidgetContainerTitle(ContainerConfigAsset->GetSettingName());
				SettingsWidget->SetWidgetContainerDescription(ContainerConfigAsset->GetSettingDescription());

				if (ContainerConfigAsset->bUsePreview)
				{
					AssociatePreviewWidget = URHUIBlueprintFunctionLibrary::CreateSettingsPreview(MyHud.Get(), ContainerConfigAsset->PreviewWidget);
					if (AssociatePreviewWidget)
					{
						SettingsWidget->SetWidgetContainerPreviewWidget(AssociatePreviewWidget);
						AddPreviewWidget(AssociatePreviewWidget);
					}
				}

                AddSettingsWidget(SettingsWidget);
                SettingsWidgets.Add(SettingsWidget);
            }
        }
    }

    OnContainerConfigSet();
}

void URHSettingsContainer::ShowSettingsWidget(class URHSettingsWidget* SettingsWidget)
{
    if (SettingsWidget != nullptr)
    {
        SettingsWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        OnShowSettingsWidget(SettingsWidget);
    }
}

void URHSettingsContainer::HideSettingsWidget(class URHSettingsWidget* SettingsWidget)
{
    if (SettingsWidget != nullptr)
    {
        SettingsWidget->SetVisibility(ESlateVisibility::Collapsed);
        OnHideSettingsWidget(SettingsWidget);
    }
}
