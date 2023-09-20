// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/Display/RHSettingsInfo_Resolution.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "DataFactories/RHSettingsDataFactory.h"

URHSettingsInfo_Resolution::URHSettingsInfo_Resolution(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{

}

void URHSettingsInfo_Resolution::InitializeValue_Implementation()
{
    Super::InitializeValue_Implementation();

    FIntPoint CurrentScreenResolution;
    CurrentScreenResolution.X = GSystemResolution.ResX;
    CurrentScreenResolution.Y = GSystemResolution.ResY;

    FScreenResolutionArray ScreenResolutionArray;

    if (RHIGetAvailableResolutions(ScreenResolutionArray, true))
    {
        ResolutionSettings.Empty();

        TArray<FIntPoint> Resolutions;

        for (const auto& ScreenResolution : ScreenResolutionArray)
        {
			// Filter screen resolutions 1280x720 and greater with wider aspect ratios than 16:10
			if (ScreenResolution.Width >= 1280 && ScreenResolution.Height >= 720 &&
				(float)ScreenResolution.Width / (float)ScreenResolution.Height >= 1.6f - KINDA_SMALL_NUMBER)
			{
				ResolutionSettings.Add(FIntPoint(ScreenResolution.Width, ScreenResolution.Height));
			}
        }

        ResolutionSettings.AddUnique(CurrentScreenResolution);

        ResolutionSettings.Sort([](const FIntPoint& a, const FIntPoint& b) { return (a.X == b.X) ? (a.Y < b.Y) : (a.X < b.X); });

        for (const auto& Resolution : ResolutionSettings)
        {
            TextOptions.Add(FText::FromString(FString::Printf(L"%d x %d", Resolution.X, Resolution.Y)));
        }
    }

    const int32 CurrentResolutionIndex = ResolutionSettings.Find(CurrentScreenResolution);

    OnValueIntSaved(CurrentResolutionIndex);

    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        if (class URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
        {
            pRHSettingsDataFactory->OnScreenResolutionApplied.AddDynamic(this, &URHSettingsInfo_Resolution::OnScreenResolutionApplied);
            pRHSettingsDataFactory->OnScreenResolutionSaved.AddDynamic(this, &URHSettingsInfo_Resolution::OnScreenResolutionSaved);
        }
    }
}

bool URHSettingsInfo_Resolution::ApplyIntValue_Implementation(int32 InInt)
{
    if (ResolutionSettings.IsValidIndex(InInt))
    {
        if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
        {
            if (class URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
            {
                pRHSettingsDataFactory->ApplyScreenResolution(ResolutionSettings[InInt]);
                return true;
            }
        }
    }
    return false;
}

bool URHSettingsInfo_Resolution::SaveIntValue_Implementation(int32 InInt)
{
    if (ResolutionSettings.IsValidIndex(InInt))
    {
        if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
        {
            if (class URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
            {
                pRHSettingsDataFactory->SaveScreenResolution();
                return true;
            }
        }
    }
    return false;
}

void URHSettingsInfo_Resolution::RevertSettingToDefault_Implementation()
{
	if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
		{
			pRHSettingsDataFactory->RevertScreenResolution();
		}
	}
}

void URHSettingsInfo_Resolution::OnScreenResolutionApplied(FIntPoint AppliedScreenResolution)
{
    if (ResolutionSettings.Contains(AppliedScreenResolution))
    {
        OnValueIntApplied(ResolutionSettings.Find(AppliedScreenResolution));
    }
}

void URHSettingsInfo_Resolution::OnScreenResolutionSaved(FIntPoint SavedScreenResolution)
{
    if (ResolutionSettings.Contains(SavedScreenResolution))
    {
        OnValueIntSaved(ResolutionSettings.Find(SavedScreenResolution));
    }
}
