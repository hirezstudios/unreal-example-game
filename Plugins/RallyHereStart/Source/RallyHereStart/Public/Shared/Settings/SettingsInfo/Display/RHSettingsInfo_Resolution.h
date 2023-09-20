// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsInfo_Resolution.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsInfo_Resolution : public URHSettingsInfoBase
{
    GENERATED_BODY()

public:
    URHSettingsInfo_Resolution(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void InitializeValue_Implementation() override;
	virtual void RevertSettingToDefault_Implementation() override;

protected:
    virtual bool ApplyIntValue_Implementation(int32 InInt) override;
    virtual bool SaveIntValue_Implementation(int32 InInt) override;

private:
    TArray<FIntPoint> ResolutionSettings;

private:
    UFUNCTION()
    void OnScreenResolutionApplied(FIntPoint AppliedScreenResolution);
    UFUNCTION()
    void OnScreenResolutionSaved(FIntPoint SavedScreenResolution);
};
