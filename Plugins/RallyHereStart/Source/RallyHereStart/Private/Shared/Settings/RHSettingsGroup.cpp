// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/RHSettingsGroup.h"
#include "Shared/Settings/RHSettingsContainer.h"
#include "RHUIBlueprintFunctionLibrary.h"

URHSettingsGroup::URHSettingsGroup(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void URHSettingsGroup::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

bool URHSettingsGroup::OnSettingsStateChanged(const FRHSettingsState& SettingsState)
{
    bool bContainsAnyContainerForInput = false;
    for (const auto& SettingsContainer : SettingsContainers)
    {
        if (SettingsContainer->OnSettingsStateChanged(SettingsState))
        {
            ShowContainer(SettingsContainer);
            bContainsAnyContainerForInput = true;
        }
        else
        {
            HideContainer(SettingsContainer);
        }
    }

    // Don't allow showing this group if our main container cannot be shown.
    if (SettingsContainers.Num() > 0 && !SettingsContainers[0]->OnSettingsStateChanged(SettingsState))
    {
        return false;
    }

    return bContainsAnyContainerForInput;
}

void URHSettingsGroup::SetGroupConfig(const struct FRHSettingsGroupConfig& InGroupConfig)
{
    GroupConfig = InGroupConfig;

    if (MyHud.IsValid())
    {
		if (const URHSettingsContainerConfigAsset* const MainSettingContainerConfig = GroupConfig.MainSettingContainerAsset)
		{
			if (URHSettingsGroup::IsContainerConfigAllowed(MainSettingContainerConfig))
			{
				if (class URHSettingsContainer* const MainSettingsContainer = CreateWidget<URHSettingsContainer>(GetWorld(), SettingsContainerClass))
				{
					MainSettingsContainer->InitializeWidget();
					MainSettingsContainer->SetContainerConfig(MainSettingContainerConfig);
					AddMainSettingsContainerWidget(MainSettingsContainer);
					SettingsContainers.Add(MainSettingsContainer);
				}

				for (const URHSettingsContainerConfigAsset* const SubSettingContainerConfig : GroupConfig.SubSettingContainerAssets)
				{
					if (URHSettingsGroup::IsContainerConfigAllowed(SubSettingContainerConfig))
					{
						if (class URHSettingsContainer* const SubSettingsContainer = CreateWidget<URHSettingsContainer>(GetWorld(), SettingsContainerClass))
						{
							SubSettingsContainer->InitializeWidget();
							SubSettingsContainer->SetContainerConfig(SubSettingContainerConfig);
							AddSubSettingsContainerWidget(SubSettingsContainer);
							SettingsContainers.Add(SubSettingsContainer);
						}
					}
				}
			}
		}
    }

    OnGroupConfigSet();
}

void URHSettingsGroup::ShowContainer(class URHSettingsContainer* SettingsContainer)
{
    if (SettingsContainer != nullptr)
    {
        SettingsContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        OnShowContainer(SettingsContainer);
    }
}

void URHSettingsGroup::HideContainer(class URHSettingsContainer* SettingsContainer)
{
    if (SettingsContainer != nullptr)
    {
        SettingsContainer->SetVisibility(ESlateVisibility::Collapsed);
        OnHideContainer(SettingsContainer);
    }
}

bool URHSettingsGroup::IsContainerConfigAllowed(const class URHSettingsContainerConfigAsset* ContainerConfig)
{
	if (ContainerConfig == nullptr)
	{
		return false;
	}

	bool bContainerAllowed = ContainerConfig->AllowedPlatformTypes.IsPlatformAllowed(UGameplayStatics::GetPlatformName());

	return bContainerAllowed;
}
