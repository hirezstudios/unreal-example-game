// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsInfo_Region.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsInfo_Region : public URHSettingsInfoBase
{
	GENERATED_BODY()

public:
    URHSettingsInfo_Region(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void InitializeValue_Implementation() override;

    void PopulateRegions();

    UFUNCTION()
    void OnPreferredRegionUpdated();

protected:
    virtual bool IsValidValueInt_Implementation(int32 InInt) const override;
    virtual int32 FixupInvalidInt_Implementation(int32 InInt) const override;

    virtual bool ApplyIntValue_Implementation(int32 InInt) override;
    virtual bool SaveIntValue_Implementation(int32 InInt) override;
};
