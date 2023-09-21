// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/Audio/RHSettingsInfo_MuteAudio.h"
#include "GameFramework/RHGameUserSettings.h"

URHSettingsInfo_MuteAudio::URHSettingsInfo_MuteAudio(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    TextOptions.Add(NSLOCTEXT("RHSettings", "Off", "Off"));
    TextOptions.Add(NSLOCTEXT("RHSettings", "On", "On"));
}

void URHSettingsInfo_MuteAudio::InitializeValue_Implementation()
{
    if (UGameUserSettings* const pGameUserSettings = GEngine->GetGameUserSettings())
    {
        if (URHGameUserSettings* const pRHGameUserSettings = Cast<URHGameUserSettings>(pGameUserSettings))
        {
			float OutFloatValue;
			pRHGameUserSettings->GetSettingAsFloat(TEXT("MasterSoundVolume"), OutFloatValue);
            if (OutFloatValue > 0.f)
            {
                OnValueIntSaved(1);
            }
            else
            {
                OnValueIntSaved(0);
            }
        }
    }
}

bool URHSettingsInfo_MuteAudio::IsValidValueInt_Implementation(int32 InInt) const
{
    return TextOptions.IsValidIndex(InInt);
}

int32 URHSettingsInfo_MuteAudio::FixupInvalidInt_Implementation(int32 InInt) const
{
    const int32 NumTextOptions = TextOptions.Num();
    if (NumTextOptions > 0)
    {
        InInt %= NumTextOptions;
        if (InInt < 0)
        {
            InInt += NumTextOptions;
        }
    }
    return InInt;
}

bool URHSettingsInfo_MuteAudio::ApplyIntValue_Implementation(int32 InInt)
{
    if (GEngine == nullptr)
    {
        return false;
    }

    if (UGameUserSettings* const pGameUserSettings = GEngine->GetGameUserSettings())
    {
        if (URHGameUserSettings* const pRHGameUserSettings = Cast<URHGameUserSettings>(pGameUserSettings))
        {
            pRHGameUserSettings->ApplySettingAsFloat(TEXT("MasterSoundVolume"), InInt == 0 ? 0.f : 1.f);
            OnValueIntApplied(InInt);
            return true;
        }
    }
    return true;
}
