// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "Shared/HUD/RHHUDCommon.h"

URHSettingsInfoBase::URHSettingsInfoBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsAutoApplied(true)
    , bIsAutoSaved(false)
    , Value(FSettingsUnion(0))
    , DirtyValue(FSettingsUnion(0))
    , RevertValue(FSettingsUnion(0))
	, PreviewValue(FSettingsUnion(0))
    , MinValue(0.f)
    , MaxValue(100.f)
    , StepValue(1.f)
    , bRoundValue(false)
    , RoundToNearest(1.f)
    , bIsPercent(false)
{
}

URHSettingsInfoBase* URHSettingsInfoBase::CreateRHSettingsInfo(class UObject* Outer, class UClass* Class)
{
    if (Outer != nullptr && Class != nullptr)
    {
        if (class URHSettingsInfoBase* SettingsInfo = NewObject<URHSettingsInfoBase>(Outer, Class))
        {
            SettingsInfo->InitializeValue();
            return SettingsInfo;
        }
    }
    return nullptr;
}

ERHSettingType URHSettingsInfoBase::GetSettingType() const
{
	if (DirtyValue.HasSubtype<bool>())
	{
		return ERHSettingType::Bool;
	}
	else if (DirtyValue.HasSubtype<int32>())
	{
		return ERHSettingType::Int;
	}
	else if (DirtyValue.HasSubtype<float>())
	{
		return ERHSettingType::Float;
	}
	else if (DirtyValue.HasSubtype<FRHKeyGroup>())
	{
		return ERHSettingType::Key;
	}

	return ERHSettingType::Invalid;
}

bool URHSettingsInfoBase::SetDesiredValueBool(bool InBool)
{
    if (IsValidValueBool(InBool))
    {
        SetDesiredValue(FSettingsUnion(InBool));
        return true;
    }
    const bool FixupBool = FixupInvalidBool(InBool);
    if (IsValidValueBool(FixupBool))
    {
        SetDesiredValue(FSettingsUnion(FixupBool));
        return true;
    }
    return false;
}

bool URHSettingsInfoBase::SetDesiredValueInt(int32 InInt)
{
    if (IsValidValueInt(InInt))
    {
        SetDesiredValue(FSettingsUnion(InInt));
        return true;
    }
    const int32 FixupInt = FixupInvalidInt(InInt);
    if (IsValidValueInt(FixupInt))
    {
        SetDesiredValue(FSettingsUnion(FixupInt));
        return true;
    }
    return false;
}

bool URHSettingsInfoBase::SetDesiredValueFloat(float InFloat)
{
    if (IsValidValueFloat(InFloat))
    {
        SetDesiredValue(FSettingsUnion(InFloat));
        return true;
    }
    const float FixupFloat = FixupInvalidFloat(InFloat);
    if (IsValidValueFloat(FixupFloat))
    {
        SetDesiredValue(FSettingsUnion(FixupFloat));
        return true;
    }
    return false;
}

bool URHSettingsInfoBase::SetDesiredValueKeyBind(FRHKeyBind InKeyBind)
{
    if (IsValidValueKeyBind(InKeyBind))
    {
        SetDesiredValue(FSettingsUnion(RHKeyBindToRHKeyGroup(InKeyBind)));
        return true;
    }
    const FRHKeyBind FixupKeyBind = FixupInvalidKeyBind(InKeyBind);
    if (IsValidValueKeyBind(FixupKeyBind))
    {
        SetDesiredValue(FSettingsUnion(RHKeyBindToRHKeyGroup(FixupKeyBind)));
        return true;
    }
    return false;
}

void URHSettingsInfoBase::SetDesiredValue(FSettingsUnion InValue)
{
	DirtyValue = InValue;
	AttemptAutoApplyAndSave();
}

bool URHSettingsInfoBase::SetPreviewValueBool(bool InBool)
{
	if (IsValidValueBool(InBool))
	{
		SetPreviewValue(FSettingsUnion(InBool));
		return true;
	}
	const bool FixupBool = FixupInvalidBool(InBool);
	if (IsValidValueBool(FixupBool))
	{
		SetPreviewValue(FSettingsUnion(FixupBool));
		return true;
	}
	return false;
}

bool URHSettingsInfoBase::SetPreviewValueInt(int32 InInt)
{
	if (IsValidValueInt(InInt))
	{
		SetPreviewValue(FSettingsUnion(InInt));
		return true;
	}
	const int32 FixupInt = FixupInvalidInt(InInt);
	if (IsValidValueInt(FixupInt))
	{
		SetPreviewValue(FSettingsUnion(FixupInt));
		return true;
	}
	return false;
}

bool URHSettingsInfoBase::SetPreviewValueFloat(float InFloat)
{
	if (IsValidValueFloat(InFloat))
	{
		SetPreviewValue(FSettingsUnion(InFloat));
		return true;
	}
	const float FixupFloat = FixupInvalidFloat(InFloat);
	if (IsValidValueFloat(FixupFloat))
	{
		SetPreviewValue(FSettingsUnion(FixupFloat));
		return true;
	}
	return false;
}

void URHSettingsInfoBase::SetPreviewValue(FSettingsUnion InValue)
{
	PreviewValue = InValue;
	ApplyPreview();
}

void URHSettingsInfoBase::ApplyPreview()
{
	// || Order is important, we want all of the ApplyTypeValue to be called no matter what.
	bool bApplied = false;
	if (PreviewValue.HasSubtype<bool>())
	{
		bApplied = ApplyPreviewBoolValue(PreviewValue.GetSubtype<bool>()) || bApplied;
	}
	if (PreviewValue.HasSubtype<int32>())
	{
		bApplied = ApplyPreviewIntValue(PreviewValue.GetSubtype<int32>()) || bApplied;
	}
	if (PreviewValue.HasSubtype<float>())
	{
		bApplied = ApplyPreviewFloatValue(PreviewValue.GetSubtype<float>()) || bApplied;
	}

	if(bApplied)
	{
		OnSettingPreviewChanged.Broadcast();
	}
}

void URHSettingsInfoBase::ResetPreview()
{
	// reset preview back to the saved applied value
	SetPreviewValue(Value);
}

void URHSettingsInfoBase::Apply()
{
    if (IsDirty())
    {
        // || Order is important, we want all of the ApplyTypeValue to be called no matter what.
        bool bApplied = false;
        if (DirtyValue.HasSubtype<bool>())
        {
            bApplied = ApplyBoolValue(DirtyValue.GetSubtype<bool>()) || bApplied;
        }
        if (DirtyValue.HasSubtype<int32>())
        {
            bApplied = ApplyIntValue(DirtyValue.GetSubtype<int32>()) || bApplied;
        }
        if (DirtyValue.HasSubtype<float>())
        {
            bApplied = ApplyFloatValue(DirtyValue.GetSubtype<float>()) || bApplied;
        }
        if (DirtyValue.HasSubtype<FRHKeyGroup>())
        {
            bApplied = ApplyKeyBindValue(RHKeyGroupToRHKeyBind(DirtyValue.GetSubtype<FRHKeyGroup>())) || bApplied;
        }

        if (bApplied)
        {
            Value = DirtyValue;
            bNeedsSave = true;
        }
    }
}

void URHSettingsInfoBase::Save()
{
    if (bNeedsSave)
    {
        // || Order is important, we want all of the SaveTypeValue to be called no matter what.
        bool bSaved = false;
        if (Value.HasSubtype<bool>())
        {
            bSaved = SaveBoolValue(Value.GetSubtype<bool>()) || bSaved;
        }
        if (Value.HasSubtype<int32>())
        {
            bSaved = SaveIntValue(Value.GetSubtype<int32>()) || bSaved;
        }
        if (Value.HasSubtype<float>())
        {
            bSaved = SaveFloatValue(Value.GetSubtype<float>()) || bSaved;
        }
        if (Value.HasSubtype<FRHKeyGroup>())
        {
            bSaved = SaveKeyBindValue(RHKeyGroupToRHKeyBind(Value.GetSubtype<FRHKeyGroup>())) || bSaved;
        }

        if (bSaved)
        {
            RevertValue = Value;
            DirtyValue = Value;
            bNeedsSave = false;
        }
    }
}

void URHSettingsInfoBase::Revert()
{
    if (CanRevert())
    {
        SetDesiredValue(RevertValue);

        Apply();
        Save();

        OnSettingValueChanged.Broadcast(true);
    }
}

void URHSettingsInfoBase::OnValueApplied(FSettingsUnion AppliedValue)
{
    const bool bValueChangedExternally = !(DirtyValue == AppliedValue);
    Value = AppliedValue;
    DirtyValue = AppliedValue;
    OnSettingValueChanged.Broadcast(bValueChangedExternally);
}

void URHSettingsInfoBase::OnValueSaved(FSettingsUnion SavedValue)
{
    const bool bValueChangedExternally = !(DirtyValue == SavedValue);
    Value = SavedValue;
    DirtyValue = SavedValue;
    RevertValue = SavedValue;
    bNeedsSave = false;
    OnSettingValueChanged.Broadcast(bValueChangedExternally);
}

float URHSettingsInfoBase::RoundToNearestValueFloat(float ValueToRound) const
{
	float ValueRange = GetMax() - GetMin();
	float DisplayValueRange = GetMaxDisplay() - GetMinDisplay();

	//If this ensure statement is triggered, the Max/Min Display values probably aren't set up properly. Make sure they aren't equal to each other.
	ensure(DisplayValueRange);

	const float DisplayRoundToNearest = DisplayValueRange ? RoundToNearest * (ValueRange) / (DisplayValueRange) : RoundToNearest;

    if (DisplayRoundToNearest > 0.f)
    {
        const float Remainder = FMath::Abs(FMath::Fmod(ValueToRound, DisplayRoundToNearest));
        const float Quotient = ValueToRound / DisplayRoundToNearest;
        const int32 RoundedToNearest = (Remainder < (DisplayRoundToNearest / 2.f)) ? FMath::FloorToInt(FMath::Abs(Quotient)) : FMath::CeilToInt(FMath::Abs(Quotient));
        return DisplayRoundToNearest * RoundedToNearest * (Quotient < 0.f ? -1.f : 1.f);
    }
    else
    {
        return ValueToRound;
    }
}

class ARHHUDCommon* URHSettingsInfoBase::GetRHHUD() const
{
    if (class APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
    {
        return Cast<class ARHHUDCommon>(PlayerController->GetHUD());
    }
    return nullptr;
}

void URHSettingsInfoBase::UpdateTextOptions(const TArray<FText>& NewOptions)
{
    if (NewOptions.Num() > 0)
    {
        TextOptions = NewOptions;
        OnTextOptionsChanged.Broadcast();
    }
}

FRHKeyGroup URHSettingsInfoBase::RHKeyBindToRHKeyGroup(const FRHKeyBind& KeyBindToConvert)
{
    TArray<FKey> OutKeys;
    EKeys::GetAllKeys(OutKeys);

    int32 OutPrimaryIndex = 0;
    OutKeys.Find(KeyBindToConvert.Primary, OutPrimaryIndex);
	const int32 PrimaryInputActionType = (int32)KeyBindToConvert.PrimaryInputActionType;

    int32 OutSecondaryIndex = 0;
    OutKeys.Find(KeyBindToConvert.Secondary, OutSecondaryIndex);
	const int32 SecondaryInputActionType = (int32)KeyBindToConvert.SecondaryInputActionType;

    int32 OutGamepadIndex = 0;
    OutKeys.Find(KeyBindToConvert.Gamepad, OutGamepadIndex);

    int32 OutComboIndex = 0;
    OutKeys.Find(KeyBindToConvert.Combo, OutComboIndex);
	const int32 GamepadInputActionType = (int32)KeyBindToConvert.GamepadInputActionType;

    return FRHKeyGroup(OutPrimaryIndex, PrimaryInputActionType, OutSecondaryIndex, SecondaryInputActionType, OutGamepadIndex, OutComboIndex, GamepadInputActionType);
}

FRHKeyBind URHSettingsInfoBase::RHKeyGroupToRHKeyBind(const FRHKeyGroup& KeyGroupToConvert)
{
    TArray<FKey> OutKeys;
    EKeys::GetAllKeys(OutKeys);

    FKey PrimaryKey = EKeys::Invalid;
	const EInputActionType PrimaryInputActionType = (EInputActionType)KeyGroupToConvert.PrimaryInputActionType;
    FKey SecondaryKey = EKeys::Invalid;
	const EInputActionType SecondaryInputActionType = (EInputActionType)KeyGroupToConvert.SecondaryInputActionType;
    FKey GamepadKey = EKeys::Invalid;
    FKey ComboKey = EKeys::Invalid;
	const EInputActionType GamepadInputActionType = (EInputActionType)KeyGroupToConvert.GamepadInputActionType;

    if (OutKeys.IsValidIndex(KeyGroupToConvert.PrimaryIndex)) PrimaryKey = OutKeys[KeyGroupToConvert.PrimaryIndex];
    if (OutKeys.IsValidIndex(KeyGroupToConvert.SecondaryIndex)) SecondaryKey = OutKeys[KeyGroupToConvert.SecondaryIndex];
    if (OutKeys.IsValidIndex(KeyGroupToConvert.GamepadIndex)) GamepadKey = OutKeys[KeyGroupToConvert.GamepadIndex];
    if (OutKeys.IsValidIndex(KeyGroupToConvert.ComboIndex)) ComboKey = OutKeys[KeyGroupToConvert.ComboIndex];

    return FRHKeyBind(PrimaryKey, PrimaryInputActionType, SecondaryKey, SecondaryInputActionType, GamepadKey, ComboKey, GamepadInputActionType);
}
