// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RHSettingsCallbackInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingUpdateMulticast);
DECLARE_DYNAMIC_DELEGATE(FOnSettingUpdate);

USTRUCT(BlueprintType)
struct RALLYHERESTART_API FSettingDelegateStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	FOnSettingUpdate SettingApplied;

	UPROPERTY(BlueprintReadWrite)
	FOnSettingUpdate SettingSaved;

};

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class URHSettingsCallbackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RALLYHERESTART_API IRHSettingsCallbackInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Settings")
	virtual bool GetSettingAsBool(FName Name, bool& OutBool) const PURE_VIRTUAL(IRHSettingsCallbackInterface::GetSettingAsBool, return false;)
	UFUNCTION(BlueprintCallable, Category = "Settings")
	virtual bool GetSettingAsInt(FName Name, int32& OutInt) const PURE_VIRTUAL(IRHSettingsCallbackInterface::GetSettingAsInt, return 0;)
	UFUNCTION(BlueprintCallable, Category = "Settings")
	virtual bool GetSettingAsFloat(FName Name, float& OutFloat) const PURE_VIRTUAL(IRHSettingsCallbackInterface::GetSettingAsFloat, return 0.f;)

	UFUNCTION(BlueprintCallable, Category = "Settings")
	virtual void BindSettingCallbacks(FName Name, const FSettingDelegateStruct& SettingDelegateStruct) PURE_VIRTUAL(IRHSettingsCallbackInterface::BindSettingCallback)
};

UCLASS()
class RALLYHERESTART_API URHSettingsCallbackStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Settings", meta=(WorldContext="WorldContextObject"))
	static TScriptInterface<IRHSettingsCallbackInterface> GetLocalSettingsCallbackInterface(const UObject* WorldContextObject);
	UFUNCTION(BlueprintPure, Category = "Settings")
	static TScriptInterface<IRHSettingsCallbackInterface> GetLocalSettingsCallbackInterfaceFromPlayer(class APlayerController* InPlayer);

	UFUNCTION(BlueprintPure, Category = "Settings", meta = (WorldContext = "WorldContextObject"))
	static bool GetLocalSettingAsBool(FName Name, bool& OutBool, const UObject* WorldContextObject);
	UFUNCTION(BlueprintPure, Category = "Settings")
	static bool GetLocalSettingAsBoolFromPlayer(FName Name, bool& OutBool, class APlayerController* InPlayer);

	UFUNCTION(BlueprintPure, Category = "Settings", meta = (WorldContext = "WorldContextObject"))
	static bool GetLocalSettingAsInt(FName Name, int32& OutInt, const UObject* WorldContextObject);
	UFUNCTION(BlueprintPure, Category = "Settings")
	static bool GetLocalSettingAsIntFromPlayer(FName Name, int32& OutInt, class APlayerController* InPlayer);

	UFUNCTION(BlueprintPure, Category = "Settings", meta = (WorldContext = "WorldContextObject"))
	static bool GetLocalSettingAsFloat(FName Name, float& OutFloat, const UObject* WorldContextObject);
	UFUNCTION(BlueprintPure, Category = "Settings")
	static bool GetLocalSettingAsFloatFromPlayer(FName Name, float& OutFloat, class APlayerController* InPlayer);

	UFUNCTION(BlueprintCallable, Category = "Settings", meta=(WorldContext="WorldContextObject"))
	static void BindSettingCallback(FName Name, const FSettingDelegateStruct& SettingDelegateStruct, const UObject* WorldContextObject);
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void BindSettingCallbackToPlayer(FName Name, const FSettingDelegateStruct& SettingDelegateStruct, class APlayerController* InPlayer);
};