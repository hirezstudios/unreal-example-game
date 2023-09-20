// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsInfo_Brightness.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsInfo_Brightness : public URHSettingsInfoBase
{
	GENERATED_BODY()
		
protected:
	virtual void InitializeValue_Implementation() override;
protected:
	virtual void RevertSettingToDefault_Implementation() override;

protected:

protected:
	virtual bool ApplyFloatValue_Implementation(float InFloat) override;

protected:
	virtual bool SaveFloatValue_Implementation(float InFloat) override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void OnSettingApplied();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void OnSettingSaved();

	void OnSwitchDockedOrUndocked(bool bIsDocked);

protected:
	UFUNCTION(BlueprintPure, Category = "Settings")
	URHSettingsDataFactory* GetRHSettingsDataFactory() const;
};
