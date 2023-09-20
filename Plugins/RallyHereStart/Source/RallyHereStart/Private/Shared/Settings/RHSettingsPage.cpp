// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/RHSettingsPage.h"
#include "Shared/Settings/RHSettingsSection.h"

URHSettingsPage::URHSettingsPage(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SettingsSectionClass = URHSettingsSection::StaticClass();
	PageConfigAsset = nullptr;
}

void URHSettingsPage::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

bool URHSettingsPage::OnSettingsStateChanged(const FRHSettingsState& SettingsState)
{
	bool bContainsAnySection = false;
	for (const auto& SettingsSection : SettingsSections)
	{
		if (SettingsSection->OnSettingsStateChanged(SettingsState))
		{
			ShowSection(SettingsSection);
			bContainsAnySection = true;
		}
		else
		{
			HideSection(SettingsSection);
		}
	}
	return bContainsAnySection;
}

void URHSettingsPage::SetPageConfig(const class URHSettingsPageConfigAsset* InPageConfigAsset)
{
	if (InPageConfigAsset == nullptr)
	{
		return;
	}

    PageConfigAsset = InPageConfigAsset;

    if (MyHud.IsValid())
    {
        for (const URHSettingsSectionConfigAsset* const SectionConfig : PageConfigAsset->SettingsSectionConfigs)
        {
            if (URHSettingsPage::IsSectionConfigAllowed(SectionConfig))
            {
                if (class URHSettingsSection* SettingsSection = CreateWidget<URHSettingsSection>(GetWorld(), SettingsSectionClass))
                {
                    SettingsSection->InitializeWidget();
                    SettingsSection->SetSectionConfig(SectionConfig);
                    AddSettingsSectionWidget(SettingsSection);
                    SettingsSections.Add(SettingsSection);
                }
            }
        }
    }

    OnPageConfigSet();
}

void URHSettingsPage::ShowSection(class URHSettingsSection* SettingsSection)
{
    if (SettingsSection != nullptr)
    {
        SettingsSection->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        OnShowSection(SettingsSection);
    }
}

void URHSettingsPage::HideSection(class URHSettingsSection* SettingsSection)
{
    if (SettingsSection != nullptr)
    {
        SettingsSection->SetVisibility(ESlateVisibility::Collapsed);
        OnHideSection(SettingsSection);
    }
}

bool URHSettingsPage::IsSectionConfigAllowed(const URHSettingsSectionConfigAsset* SectionConfig)
{
	if (SectionConfig == nullptr)
	{
		return false;
	}

    for (const FRHSettingsGroupConfig& GroupConfig : SectionConfig->SettingsGroups)
    {
        if (URHSettingsSection::IsGroupConfigAllowed(GroupConfig))
        {
            return true;
        }
    }
    return false;
}
