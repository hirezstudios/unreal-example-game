// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/Controls/RHSettingsInfo_GamepadIconSet.h"
#include "GameFramework/RHGameUserSettings.h"

URHSettingsInfo_GamepadIconSet::URHSettingsInfo_GamepadIconSet(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void URHSettingsInfo_GamepadIconSet::InitializeValue_Implementation()
{
	TextOptions.Add(NSLOCTEXT("RHSettings", "XboxGamepadStyle", "Xbox"));
	TextOptions.Add(NSLOCTEXT("RHSettings", "PlaystationGamepadStyle", "Playstation"));
	OnTextOptionsChanged.Broadcast();

    if (UGameUserSettings* const pGameUserSettings = GEngine->GetGameUserSettings())
    {
        if (URHGameUserSettings* const pRHGameUserSettings = Cast<URHGameUserSettings>(pGameUserSettings))
        {
			OnValueIntSaved((int32)pRHGameUserSettings->GetGamepadIconSet());
        }
    }
}

bool URHSettingsInfo_GamepadIconSet::IsValidValueInt_Implementation(int32 InInt) const
{
    return TextOptions.IsValidIndex(InInt);
}

int32 URHSettingsInfo_GamepadIconSet::FixupInvalidInt_Implementation(int32 InInt) const
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

bool URHSettingsInfo_GamepadIconSet::ApplyIntValue_Implementation(int32 InInt)
{
    if (GEngine == nullptr)
    {
        return false;
    }

    if (UGameUserSettings* const pGameUserSettings = GEngine->GetGameUserSettings())
    {
        if (URHGameUserSettings* const pRHGameUserSettings = Cast<URHGameUserSettings>(pGameUserSettings))
        {
			pRHGameUserSettings->SetGamepadIconSet(EGamepadIcons(InInt));
            OnValueIntApplied(InInt);
            return true;
        }
    }
    return true;
}
