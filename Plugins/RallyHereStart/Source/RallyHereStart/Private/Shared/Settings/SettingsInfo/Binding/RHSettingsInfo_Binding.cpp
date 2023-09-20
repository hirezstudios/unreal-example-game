// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/Binding/RHSettingsInfo_Binding.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "DataFactories/RHSettingsDataFactory.h"

URHSettingsInfo_Binding::URHSettingsInfo_Binding(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , PrimaryKeyBindInfo(FName(""), 0.f, EInputType::KBM, EKeyBindType::ActionMapping, false)
    , GamepadKeyBindInfo(FName(""), 0.f, EInputType::GP, EKeyBindType::ActionMapping, false)
{
}

void URHSettingsInfo_Binding::InitializeValue_Implementation()
{
    OnValueKeyBindSaved(GetKeyBind());

    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        if (class URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
        {
            if (!PrimaryKeyBindInfo.IsPermanentBinding && PrimaryKeyBindInfo.Name != FName())
            {
                pRHSettingsDataFactory->AddEditableKeybindName(PrimaryKeyBindInfo.Name);
            }
            if (!GamepadKeyBindInfo.IsPermanentBinding && GamepadKeyBindInfo.Name != FName())
            {
                pRHSettingsDataFactory->AddEditableKeybindName(GamepadKeyBindInfo.Name);
            }
            pRHSettingsDataFactory->OnKeyBindSettingsApplied.AddUniqueDynamic(this, &URHSettingsInfo_Binding::OnKeyBindingsApplied);
            pRHSettingsDataFactory->OnKeyBindSettingsSaved.AddUniqueDynamic(this, &URHSettingsInfo_Binding::OnKeyBindingsSaved);

			pRHSettingsDataFactory->OnSettingsReceivedFromPlayerAccount.AddUniqueDynamic(this, &URHSettingsInfo_Binding::OnSettingsReceivedFromPlayerAccount);
        }
    }
}

void URHSettingsInfo_Binding::OnKeyBindingsApplied(FName Name)
{
    if (PrimaryKeyBindInfo.Name == Name || GamepadKeyBindInfo.Name == Name)
    {
        OnValueKeyBindApplied(GetKeyBind());
    }
}

void URHSettingsInfo_Binding::OnKeyBindingsSaved(FName Name)
{
    if (PrimaryKeyBindInfo.Name == Name || GamepadKeyBindInfo.Name == Name)
    {
        OnValueKeyBindSaved(GetKeyBind());
    }
}

FRHKeyBind URHSettingsInfo_Binding::GetKeyBind() const
{
	FRHKeyBind ToReturn;
    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        if (class URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
        {
			TArray<FRHInputActionKey> OutActionKeys;
			TArray<FKey> OutAxisKeys;

            switch (PrimaryKeyBindInfo.KeyBindType)
            {
            case EKeyBindType::ActionMapping:
                pRHSettingsDataFactory->GetCustomRHInputActionKeys(PrimaryKeyBindInfo.Name, PrimaryKeyBindInfo.InputType, OutActionKeys);
				ToReturn.Primary = OutActionKeys.IsValidIndex(0) ? OutActionKeys[0].Key : EKeys::Invalid;
				ToReturn.PrimaryInputActionType = OutActionKeys.IsValidIndex(0) ? OutActionKeys[0].Type : EInputActionType::Press;
				ToReturn.Secondary = OutActionKeys.IsValidIndex(1) ? OutActionKeys[1].Key : EKeys::Invalid;
				ToReturn.SecondaryInputActionType = OutActionKeys.IsValidIndex(1) ? OutActionKeys[1].Type : EInputActionType::Press;
                break;
            case EKeyBindType::AxisMapping:
                pRHSettingsDataFactory->GetCustomInputAxisKeys(PrimaryKeyBindInfo.Name, PrimaryKeyBindInfo.InputType, PrimaryKeyBindInfo.Scale, OutAxisKeys);
				ToReturn.Primary = OutAxisKeys.IsValidIndex(0) ? OutAxisKeys[0] : EKeys::Invalid;
				ToReturn.Secondary = OutAxisKeys.IsValidIndex(1) ? OutAxisKeys[1] : EKeys::Invalid;
                break;
            default:
                break;
            }

			OutActionKeys.Empty();
			OutAxisKeys.Empty();

			switch (GamepadKeyBindInfo.KeyBindType)
			{
			case EKeyBindType::ActionMapping:
				pRHSettingsDataFactory->GetCustomRHInputActionKeys(GamepadKeyBindInfo.Name, GamepadKeyBindInfo.InputType, OutActionKeys);
				ToReturn.Gamepad = OutActionKeys.IsValidIndex(0) ? OutActionKeys[0].Key : EKeys::Invalid;
				ToReturn.Combo = OutActionKeys.IsValidIndex(1) ? OutActionKeys[1].Key : EKeys::Invalid;
				ToReturn.GamepadInputActionType = OutActionKeys.IsValidIndex(0) ? OutActionKeys[0].Type : EInputActionType::Press;
				break;
			case EKeyBindType::AxisMapping:
				pRHSettingsDataFactory->GetCustomInputAxisKeys(GamepadKeyBindInfo.Name, GamepadKeyBindInfo.InputType, GamepadKeyBindInfo.Scale, OutAxisKeys);
				ToReturn.Gamepad = OutAxisKeys.IsValidIndex(0) ? OutAxisKeys[0] : EKeys::Invalid;
				ToReturn.Combo = OutAxisKeys.IsValidIndex(1) ? OutAxisKeys[1] : EKeys::Invalid;
				break;
			default:
				break;
			}

        }
    }
	return ToReturn;
}

bool URHSettingsInfo_Binding::ApplyKeyBindValue_Implementation(FRHKeyBind InKeyBind)
{
    bool bApplied = false;
    if (InKeyBind.Primary != GetValueKeyBind().Primary
	|| InKeyBind.Secondary != GetValueKeyBind().Secondary
	|| InKeyBind.PrimaryInputActionType != GetValueKeyBind().PrimaryInputActionType
	|| InKeyBind.SecondaryInputActionType != GetValueKeyBind().SecondaryInputActionType)
    {
		if (PrimaryKeyBindInfo.KeyBindType == EKeyBindType::ActionMapping)
		{
			TArray<FRHInputActionKey> KeysToApply;
			if (InKeyBind.Primary != EKeys::Invalid)
			{
				KeysToApply.Emplace(InKeyBind.Primary, InKeyBind.PrimaryInputActionType);
			}
			if (InKeyBind.Secondary != EKeys::Invalid)
			{
				KeysToApply.AddUnique(FRHInputActionKey(InKeyBind.Secondary, InKeyBind.SecondaryInputActionType));
			}
			bApplied = ApplyActionKeyBindInfo(PrimaryKeyBindInfo, KeysToApply) || bApplied;
		}
		else
		{
			TArray<FKey> KeysToApply;
			if (InKeyBind.Primary != EKeys::Invalid)
			{
				KeysToApply.Add(InKeyBind.Primary);
			}
			if (InKeyBind.Secondary != EKeys::Invalid)
			{
				KeysToApply.AddUnique(InKeyBind.Secondary);
			}
			bApplied = ApplyAxisKeyBindInfo(PrimaryKeyBindInfo, KeysToApply) || bApplied;
		}
    }

    if (InKeyBind.Gamepad != GetValueKeyBind().Gamepad
	|| InKeyBind.Combo != GetValueKeyBind().Combo
	|| InKeyBind.GamepadInputActionType != GetValueKeyBind().GamepadInputActionType)
    {
		if (GamepadKeyBindInfo.KeyBindType == EKeyBindType::ActionMapping)
		{
			TArray<FRHInputActionKey> KeysToApply;
			if (InKeyBind.Gamepad != EKeys::Invalid)
			{
				KeysToApply.Emplace(InKeyBind.Gamepad, InKeyBind.GamepadInputActionType);
			}
			if (InKeyBind.Combo != EKeys::Invalid)
			{
				KeysToApply.AddUnique(FRHInputActionKey(InKeyBind.Combo, InKeyBind.GamepadInputActionType));
			}
			bApplied = ApplyActionKeyBindInfo(GamepadKeyBindInfo, KeysToApply) || bApplied;
		}
		else
		{
			TArray<FKey> KeysToApply;
			if (InKeyBind.Gamepad != EKeys::Invalid)
			{
				KeysToApply.Add(InKeyBind.Gamepad);
			}
			if (InKeyBind.Combo != EKeys::Invalid)
			{
				KeysToApply.AddUnique(InKeyBind.Combo);
			}
			bApplied = ApplyAxisKeyBindInfo(GamepadKeyBindInfo, KeysToApply) || bApplied;
		}
    }
    return bApplied;
}

bool URHSettingsInfo_Binding::ApplyActionKeyBindInfo(const FRHKeyBindInfo& KeyBindInfoToApply, const TArray<FRHInputActionKey>& KeysToApply)
{
    bool bApplied = false;
    if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
    {
        if (class URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
        {
            switch (KeyBindInfoToApply.KeyBindType)
            {
            case EKeyBindType::ActionMapping:
                pRHSettingsDataFactory->SetCustomInputActionKeyMapping(KeyBindInfoToApply.Name, KeysToApply, KeyBindInfoToApply.InputType);
                bApplied = true;
                break;
            default:
                break;
            }
        }
    }
    return bApplied;
}

bool URHSettingsInfo_Binding::ApplyAxisKeyBindInfo(const FRHKeyBindInfo& KeyBindInfoToApply, const TArray<FKey>& KeysToApply)
{
	bool bApplied = false;
	if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
	{
		if (class URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
		{
			switch (KeyBindInfoToApply.KeyBindType)
			{
			case EKeyBindType::AxisMapping:
				pRHSettingsDataFactory->SetCustomInputAxisKeyMapping(KeyBindInfoToApply.Name, KeysToApply, KeyBindInfoToApply.InputType, KeyBindInfoToApply.Scale);
				bApplied = true;
				break;
			default:
				break;
			}
		}
	}
	return bApplied;
}

void URHSettingsInfo_Binding::OnSettingsReceivedFromPlayerAccount()
{
	OnValueKeyBindSaved(GetKeyBind());
}
