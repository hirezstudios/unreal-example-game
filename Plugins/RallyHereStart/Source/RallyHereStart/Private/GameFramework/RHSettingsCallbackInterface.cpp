// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#include "GameFramework/RHSettingsCallbackInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "GameFramework/RHHUDInterface.h"

#include "UnrealEngine.h"

TScriptInterface<IRHSettingsCallbackInterface> URHSettingsCallbackStatics::GetLocalSettingsCallbackInterface(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World != nullptr && !World->IsNetMode(NM_DedicatedServer))
	{
		for (TPlayerControllerIterator<APlayerController>::LocalOnly It(World); It; ++It)
		{
			check(*It);
			return GetLocalSettingsCallbackInterfaceFromPlayer(*It);
		}
	}

	return TScriptInterface<IRHSettingsCallbackInterface>();
}

TScriptInterface<IRHSettingsCallbackInterface> URHSettingsCallbackStatics::GetLocalSettingsCallbackInterfaceFromPlayer(class APlayerController* InPlayer)
{
	if (InPlayer == nullptr)
	{
		return TScriptInterface<IRHSettingsCallbackInterface>();
	}

	AHUD* pHUD = InPlayer->GetHUD();
	if (pHUD == nullptr || !pHUD->GetClass()->ImplementsInterface(URHHUDInterface::StaticClass()))
	{
		return TScriptInterface<IRHSettingsCallbackInterface>();
	}

	return IRHHUDInterface::Execute_GetSettingsCallbackInterface(pHUD);
}

bool URHSettingsCallbackStatics::GetLocalSettingAsBool(FName Name, bool& OutBool, const UObject* WorldContextObject)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterface(WorldContextObject);
	if (SettingsCallbackInterace)
	{
		return SettingsCallbackInterace->GetSettingAsBool(Name, OutBool);
	}
	return false;
}

bool URHSettingsCallbackStatics::GetLocalSettingAsBoolFromPlayer(FName Name, bool& OutBool, class APlayerController* InPlayer)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterfaceFromPlayer(InPlayer);
	if (SettingsCallbackInterace)
	{
		return SettingsCallbackInterace->GetSettingAsBool(Name, OutBool);
	}
	return false;
}

bool URHSettingsCallbackStatics::GetLocalSettingAsInt(FName Name, int32& OutInt, const UObject* WorldContextObject)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterface(WorldContextObject);
	if (SettingsCallbackInterace)
	{
		return SettingsCallbackInterace->GetSettingAsInt(Name, OutInt);
	}
	return false;
}

bool URHSettingsCallbackStatics::GetLocalSettingAsIntFromPlayer(FName Name, int32& OutInt, class APlayerController* InPlayer)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterfaceFromPlayer(InPlayer);
	if (SettingsCallbackInterace)
	{
		return SettingsCallbackInterace->GetSettingAsInt(Name, OutInt);
	}
	return false;
}

bool URHSettingsCallbackStatics::GetLocalSettingAsFloat(FName Name, float& OutFloat, const UObject* WorldContextObject)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterface(WorldContextObject);
	if (SettingsCallbackInterace)
	{
		return SettingsCallbackInterace->GetSettingAsFloat(Name, OutFloat);
	}
	return false;
}

bool URHSettingsCallbackStatics::GetLocalSettingAsFloatFromPlayer(FName Name, float& OutFloat, class APlayerController* InPlayer)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterfaceFromPlayer(InPlayer);
	if (SettingsCallbackInterace)
	{
		return SettingsCallbackInterace->GetSettingAsFloat(Name, OutFloat);
	}
	return false;
}

void URHSettingsCallbackStatics::BindSettingCallback(FName Name, const FSettingDelegateStruct& SettingDelegateStruct, const UObject* WorldContextObject)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterface(WorldContextObject);
	if (SettingsCallbackInterace)
	{
		SettingsCallbackInterace->BindSettingCallbacks(Name, SettingDelegateStruct);
	}
}

void URHSettingsCallbackStatics::BindSettingCallbackToPlayer(FName Name, const FSettingDelegateStruct& SettingDelegateStruct, APlayerController* InPlayer)
{
	TScriptInterface<IRHSettingsCallbackInterface> SettingsCallbackInterace = GetLocalSettingsCallbackInterfaceFromPlayer(InPlayer);
	if (SettingsCallbackInterace)
	{
		SettingsCallbackInterace->BindSettingCallbacks(Name, SettingDelegateStruct);
	}
}
