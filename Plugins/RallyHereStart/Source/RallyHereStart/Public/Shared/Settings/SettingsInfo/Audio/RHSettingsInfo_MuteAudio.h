// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsInfo_MuteAudio.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsInfo_MuteAudio : public URHSettingsInfoBase
{
	GENERATED_BODY()

public:
    URHSettingsInfo_MuteAudio(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void InitializeValue_Implementation() override;

public:
    virtual bool IsValidValueInt_Implementation(int32 InInt) const override;

private:
    virtual int32 FixupInvalidInt_Implementation(int32 InInt) const override;

protected:
    virtual bool ApplyIntValue_Implementation(int32 InInt) override;
};