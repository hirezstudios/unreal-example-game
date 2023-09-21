// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "RHSettingsInfo_Generic.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSettingsInfo_Generic : public URHSettingsInfoBase
{
	GENERATED_BODY()
		
protected:
	virtual void InitializeValue_Implementation() override;
protected:
	virtual void RevertSettingToDefault_Implementation() override;

protected:
	//What type of setting this is.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generic")
	ERHSettingType RHSettingType;

	//Name to read/write from the ini file in settings.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generic")
	FName Name;

protected:
	virtual bool ApplyBoolValue_Implementation(bool InBool) override;
	virtual bool ApplyIntValue_Implementation(int32 InInt) override;
	virtual bool ApplyFloatValue_Implementation(float InFloat) override;

protected:
	virtual bool SaveBoolValue_Implementation(bool InBool) override;
	virtual bool SaveIntValue_Implementation(int32 InInt) override;
	virtual bool SaveFloatValue_Implementation(float InFloat) override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void OnSettingApplied();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void OnSettingSaved();

protected:
	UFUNCTION(BlueprintPure, Category = "Settings")
	URHSettingsDataFactory* GetRHSettingsDataFactory() const;
	
};
