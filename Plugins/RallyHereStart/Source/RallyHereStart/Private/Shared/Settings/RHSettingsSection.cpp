// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/RHSettingsSection.h"
#include "Shared/Settings/RHSettingsGroup.h"

URHSettingsSection::URHSettingsSection(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SettingsGroupClass = URHSettingsGroup::StaticClass();
	SectionConfigAsset = nullptr;
}

void URHSettingsSection::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

bool URHSettingsSection::OnSettingsStateChanged(const FRHSettingsState& SettingsState)
{
    bool bContainsAnyGroup = false;
    for (const auto& SettingsGroup : SettingsGroups)
    {
        if (SettingsGroup->OnSettingsStateChanged(SettingsState))
        {
            ShowGroup(SettingsGroup);
            bContainsAnyGroup = true;
        }
        else
        {
            HideGroup(SettingsGroup);
        }
    }
    return bContainsAnyGroup;
}

void URHSettingsSection::SetSectionConfig(const class URHSettingsSectionConfigAsset* InSectionConfigAsset)
{
	if (InSectionConfigAsset == nullptr)
	{
		return;
	}

	SectionConfigAsset = InSectionConfigAsset;

    if (MyHud.IsValid())
    {
        for (const FRHSettingsGroupConfig& SettingGroupConfig : SectionConfigAsset->SettingsGroups)
        {
            if (URHSettingsSection::IsGroupConfigAllowed(SettingGroupConfig))
            {
                if (class URHSettingsGroup* SettingsGroup = CreateWidget<URHSettingsGroup>(GetWorld(), SettingsGroupClass))
                {
                    SettingsGroup->InitializeWidget();
                    SettingsGroup->SetGroupConfig(SettingGroupConfig);
                    AddSettingsGroupWidget(SettingsGroup);
                    SettingsGroups.Add(SettingsGroup);
                }
            }
        }
    }

    OnSectionConfigSet();
}

void URHSettingsSection::ShowGroup(class URHSettingsGroup* SettingsGroup)
{
    if (SettingsGroup != nullptr)
    {
        SettingsGroup->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        OnShowGroup(SettingsGroup);
    }
}

void URHSettingsSection::HideGroup(class URHSettingsGroup* SettingsGroup)
{
    if (SettingsGroup != nullptr)
    {
        SettingsGroup->SetVisibility(ESlateVisibility::Collapsed);
        OnHideGroup(SettingsGroup);
    }
}

bool URHSettingsSection::IsGroupConfigAllowed(const FRHSettingsGroupConfig& GroupConfig)
{
    return URHSettingsGroup::IsContainerConfigAllowed(GroupConfig.MainSettingContainerAsset);
}
