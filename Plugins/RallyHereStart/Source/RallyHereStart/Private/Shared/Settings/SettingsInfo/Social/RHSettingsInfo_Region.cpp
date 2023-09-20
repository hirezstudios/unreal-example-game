// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/Social/RHSettingsInfo_Region.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "DataFactories/RHSettingsDataFactory.h"

URHSettingsInfo_Region::URHSettingsInfo_Region(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{

}

void URHSettingsInfo_Region::InitializeValue_Implementation()
{
    Super::InitializeValue_Implementation();
   
    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        OnPreferredRegionUpdated();

        pRHHUD->OnPreferredRegionUpdated.AddUniqueDynamic(this, &URHSettingsInfo_Region::OnPreferredRegionUpdated);

        if (URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
        {
            pRHSettingsDataFactory->OnSettingsReceivedFromPlayerAccount.AddUniqueDynamic(this, &URHSettingsInfo_Region::OnPreferredRegionUpdated);
        }
    }
}

void URHSettingsInfo_Region::PopulateRegions()
{
    TextOptions.Empty();
    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        TMap<FString, FText> OutRegionIdToNameMap;
        pRHHUD->GetRegionList(OutRegionIdToNameMap);
        for (const auto& Pair : OutRegionIdToNameMap)
        {
            TextOptions.Add(Pair.Value);
        }
    }
    OnTextOptionsChanged.Broadcast();
}

void URHSettingsInfo_Region::OnPreferredRegionUpdated()
{
    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        PopulateRegions();

        TMap<FString, FText> OutRegionIdToNameMap;
        pRHHUD->GetRegionList(OutRegionIdToNameMap);
        TArray<FString> Keys;
		OutRegionIdToNameMap.GenerateKeyArray(Keys);
        for (int32 i = 0; i < Keys.Num(); ++i)
        {
            FString RegionId;
            if (pRHHUD->GetPreferredRegionId(RegionId) && RegionId == Keys[i])
            {
                OnValueIntSaved(i);
                OnSettingValueChanged.Broadcast(true);
            }
        }
    }
}

bool URHSettingsInfo_Region::IsValidValueInt_Implementation(int32 InInt) const
{
    return TextOptions.IsValidIndex(InInt);
}

int32 URHSettingsInfo_Region::FixupInvalidInt_Implementation(int32 InInt) const
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

bool URHSettingsInfo_Region::ApplyIntValue_Implementation(int32 InInt)
{
    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        TMap<FString, FText> OutRegionIdToNameMap;
        pRHHUD->GetRegionList(OutRegionIdToNameMap);
        TArray<FString> Keys;
		OutRegionIdToNameMap.GenerateKeyArray(Keys);
        if (OutRegionIdToNameMap.Contains(Keys[InInt]))
        {
            pRHHUD->OnPreferredRegionUpdated.RemoveAll(this);
            pRHHUD->SetPreferredRegionId(Keys[InInt]);
            pRHHUD->OnPreferredRegionUpdated.AddUniqueDynamic(this, &URHSettingsInfo_Region::OnPreferredRegionUpdated);
            return true;
        }
    }
    return false;
}

bool URHSettingsInfo_Region::SaveIntValue_Implementation(int32 InInt)
{
    return true;
}
