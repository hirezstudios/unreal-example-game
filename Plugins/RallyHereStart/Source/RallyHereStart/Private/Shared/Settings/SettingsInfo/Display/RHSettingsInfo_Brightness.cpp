// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/Display/RHSettingsInfo_Brightness.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "GameFramework/RHGameUserSettings.h"
#if defined(PLATFORM_SWITCH) && PLATFORM_SWITCH
#include "SwitchApplication.h"
#endif

static const FName& GetBrightnessSetting()
{
#if defined(PLATFORM_SWITCH) && PLATFORM_SWITCH
	if (!FSwitchApplication::IsBoostModeActive())
	{
		return URHGameUserSettings::BrightnessHandheld;
	}
#endif
	return URHGameUserSettings::Brightness;
}

void URHSettingsInfo_Brightness::InitializeValue_Implementation()
{
	Super::InitializeValue_Implementation();

	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		{
			FSettingDelegateStruct SettingDelegateStruct;
			SettingDelegateStruct.SettingApplied.BindUFunction(this, FName("OnSettingApplied"));
			SettingDelegateStruct.SettingSaved.BindUFunction(this, FName("OnSettingSaved"));
			pRHSettingsDataFactory->BindSettingCallbacks(URHGameUserSettings::Brightness, SettingDelegateStruct);
		}

		{
			FSettingDelegateStruct SettingDelegateStruct;
			SettingDelegateStruct.SettingApplied.BindUFunction(this, FName("OnSettingApplied"));
			SettingDelegateStruct.SettingSaved.BindUFunction(this, FName("OnSettingSaved"));
			pRHSettingsDataFactory->BindSettingCallbacks(URHGameUserSettings::BrightnessHandheld, SettingDelegateStruct);
		}

		// It's possible that the setting is loaded without notify (console GameSaveData).
		// After the settings are received from the account, go ahead and update our state to match.
		pRHSettingsDataFactory->OnSettingsReceivedFromPlayerAccount.AddUniqueDynamic(this, &URHSettingsInfo_Brightness::OnSettingSaved);

#if defined(PLATFORM_SWITCH) && PLATFORM_SWITCH
		FSwitchApplication::OnSwitchDockedOrUndocked.RemoveAll(this);
		FSwitchApplication::OnSwitchDockedOrUndocked.AddUObject(this, &URHSettingsInfo_Brightness::OnSwitchDockedOrUndocked);
#endif

		float BrightnessValue;
		if (pRHSettingsDataFactory->GetSettingAsFloat(GetBrightnessSetting(), BrightnessValue))
		{
			OnValueFloatSaved(BrightnessValue);
		}
	}
}

void URHSettingsInfo_Brightness::OnSwitchDockedOrUndocked(bool bIsDocked)
{
	OnSettingSaved();
}

void URHSettingsInfo_Brightness::RevertSettingToDefault_Implementation()
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		pRHSettingsDataFactory->RevertSettingToDefault(GetBrightnessSetting());
	}
}

bool URHSettingsInfo_Brightness::ApplyFloatValue_Implementation(float InFloat)
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		pRHSettingsDataFactory->ApplySettingAsFloat(GetBrightnessSetting(), InFloat);
		return true;
	}
	return false;
}

bool URHSettingsInfo_Brightness::SaveFloatValue_Implementation(float InFloat)
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		pRHSettingsDataFactory->SaveSettingAsFloat(GetBrightnessSetting());
		return true;
	}
	return false;
}

void URHSettingsInfo_Brightness::OnSettingApplied()
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		float OutDesiredFloat;
		if (pRHSettingsDataFactory->GetSettingAsFloat(GetBrightnessSetting(), OutDesiredFloat))
		{
			OnValueApplied(FSettingsUnion(OutDesiredFloat));
		}
	}
}

void URHSettingsInfo_Brightness::OnSettingSaved()
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		float OutDesiredFloat;
		if (pRHSettingsDataFactory->GetSettingAsFloat(GetBrightnessSetting(), OutDesiredFloat))
		{
			OnValueSaved(FSettingsUnion(OutDesiredFloat));
		}
	}
}

URHSettingsDataFactory* URHSettingsInfo_Brightness::GetRHSettingsDataFactory() const
{
	if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
		{
			return pRHSettingsDataFactory;
		}
	}
	return nullptr;
}
