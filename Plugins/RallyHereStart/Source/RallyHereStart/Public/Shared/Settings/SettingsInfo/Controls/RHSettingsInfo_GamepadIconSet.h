// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsInfo_GamepadIconSet.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsInfo_GamepadIconSet : public URHSettingsInfoBase
{
	GENERATED_BODY()

public:
	URHSettingsInfo_GamepadIconSet(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void InitializeValue_Implementation() override;

public:
    virtual bool IsValidValueInt_Implementation(int32 InInt) const override;

private:
    virtual int32 FixupInvalidInt_Implementation(int32 InInt) const override;

protected:
    virtual bool ApplyIntValue_Implementation(int32 InInt) override;
};
