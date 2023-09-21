// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Containers/Union.h"
#include "InputCoreTypes.h"
#include "GameFramework/RHPlayerInput.h"
#include "RHSettingsInfo.generated.h"

UENUM(BlueprintType)
enum class ERHSettingType : uint8
{
	Bool	UMETA(DisplayName = "Bool"),
	Int		UMETA(DisplayName = "Int"),
	Float	UMETA(DisplayName = "Float"),
	Key		UMETA(DisplayName = "Key"),
	Invalid	UMETA(DisplayName = "Invalid")
};

USTRUCT(BlueprintType)
struct FRHKeyBind
{
    GENERATED_USTRUCT_BODY()

public:
    FRHKeyBind()
        : Primary(EKeys::Invalid)
		, PrimaryInputActionType(EInputActionType::Press)
        , Secondary(EKeys::Invalid)
		, SecondaryInputActionType(EInputActionType::Press)
        , Gamepad(EKeys::Invalid)
        , Combo(EKeys::Invalid)
		, GamepadInputActionType(EInputActionType::Press)
    {}

    FRHKeyBind(FKey InPrimary, EInputActionType InPrimaryInputActionType, FKey InSecondary, EInputActionType InSecondaryInputActionType, FKey InGamepad, FKey InCombo, EInputActionType InGamepadInputActionType)
        : Primary(InPrimary)
		, PrimaryInputActionType(InPrimaryInputActionType)
        , Secondary(InSecondary)
		, SecondaryInputActionType(InSecondaryInputActionType)
        , Gamepad(InGamepad)
        , Combo(InCombo)
		, GamepadInputActionType(InGamepadInputActionType)
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FKey Primary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	EInputActionType PrimaryInputActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FKey Secondary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	EInputActionType SecondaryInputActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FKey Gamepad;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
    FKey Combo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	EInputActionType GamepadInputActionType;
};

USTRUCT()
struct FRHKeyGroup
{
    GENERATED_USTRUCT_BODY()

public:
    FRHKeyGroup()
        : PrimaryIndex(0)
		, PrimaryInputActionType(0)
        , SecondaryIndex(0)
		, SecondaryInputActionType(0)
        , GamepadIndex(0)
        , ComboIndex(0)
		, GamepadInputActionType(0)
    {}

    FRHKeyGroup(int32 InPrimaryIndex, int32 InPrimaryInputActionType, int32 InSecondaryIndex, int32 InSecondaryInputActionType, int32 InGamepadIndex, int32 InComboIndex, int32 InGamepadInputActionType)
        : PrimaryIndex(InPrimaryIndex)
		, PrimaryInputActionType(InPrimaryInputActionType)
        , SecondaryIndex(InSecondaryIndex)
		, SecondaryInputActionType(InSecondaryInputActionType)
        , GamepadIndex(InGamepadIndex)
        , ComboIndex(InComboIndex)
		, GamepadInputActionType(InGamepadInputActionType)
    {}

    int32 PrimaryIndex;
	int32 PrimaryInputActionType;
    int32 SecondaryIndex;
	int32 SecondaryInputActionType;
    int32 GamepadIndex;
    int32 ComboIndex;
	int32 GamepadInputActionType;

public:
    friend bool operator==(const FRHKeyGroup& KeyGroupA, const FRHKeyGroup& KeyGroupB)
    {
        const bool PrimaryEquals = KeyGroupA.PrimaryIndex == KeyGroupB.PrimaryIndex;
		const bool PrimaryInputActionTypeEquals = KeyGroupA.PrimaryInputActionType == KeyGroupB.PrimaryInputActionType;
        const bool SecondaryEquals = KeyGroupA.SecondaryIndex == KeyGroupB.SecondaryIndex;
		const bool SecondaryInputActionTypeEquals = KeyGroupA.SecondaryInputActionType == KeyGroupB.SecondaryInputActionType;
        const bool GamepadEquals = KeyGroupA.GamepadIndex == KeyGroupB.GamepadIndex;
        const bool ComboEquals = KeyGroupA.ComboIndex == KeyGroupB.ComboIndex;
		const bool GamepadInputActionTypeEquals = KeyGroupA.GamepadInputActionType == KeyGroupB.GamepadInputActionType;
        return PrimaryEquals && PrimaryInputActionTypeEquals && SecondaryEquals && SecondaryInputActionTypeEquals && GamepadEquals && ComboEquals && GamepadInputActionTypeEquals;
    }
    friend bool operator!=(const FRHKeyGroup& KeyGroupA, const FRHKeyGroup& KeyGroupB) { return !(KeyGroupA == KeyGroupB); }
};

typedef TUnion<bool, int32, float, FRHKeyGroup> FSettingsUnion;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingValueChanged, bool, ChangedExternally);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTextOptionsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingPreviewChanged);

/**
 * 
 */
UCLASS(Blueprintable)
class RALLYHERESTART_API URHSettingsInfoBase : public UObject
{
	GENERATED_BODY()

public:
    URHSettingsInfoBase(const FObjectInitializer& ObjectInitializer);

    static URHSettingsInfoBase* CreateRHSettingsInfo(UObject* Outer, UClass* Class);

// Initial Value: Get what our Value should start off as.
protected:
    UFUNCTION(BlueprintNativeEvent, Category = "Settings")
    void InitializeValue();
    virtual void InitializeValue_Implementation() {}

	// Default: Try to set the setting info back to the default.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Settings")
	void RevertSettingToDefault();
	virtual void RevertSettingToDefault_Implementation() {}

public:
	UFUNCTION(BlueprintPure, Category = "Settings")
	ERHSettingType GetSettingType() const;

// Valid Value: Override to disallow dirtying of certain values from SetDesiredValue.
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Bool")
    bool IsValidValueBool(bool InBool) const;
    virtual bool IsValidValueBool_Implementation(bool InBool) const { return true; }
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Int")
    bool IsValidValueInt(int32 InInt) const;
    virtual bool IsValidValueInt_Implementation(int32 InInt) const { return true; }
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Float")
    bool IsValidValueFloat(float InFloat) const;
    virtual bool IsValidValueFloat_Implementation(float InFloat) const { return true; }
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Key")
    bool IsValidValueKeyBind(FRHKeyBind InKey) const;
    virtual bool IsValidValueKeyBind_Implementation(FRHKeyBind InKeyBind) const { return true; }

protected:
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Bool")
    bool FixupInvalidBool(bool InBool) const;
    virtual bool FixupInvalidBool_Implementation(bool InBool) const { return InBool; }
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Int")
    int32 FixupInvalidInt(int32 InInt) const;
    virtual int32 FixupInvalidInt_Implementation(int32 InInt) const { return InInt; }
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Float")
    float FixupInvalidFloat(float InFloat) const;
    virtual float FixupInvalidFloat_Implementation(float InFloat) const { return InFloat; }
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Key")
    FRHKeyBind FixupInvalidKeyBind(FRHKeyBind InKey) const;
    virtual FRHKeyBind FixupInvalidKeyBind_Implementation(FRHKeyBind InKeyBind) const { return InKeyBind; }

// Set Options: These are NOT overridable. Sets the dirty value.
public:
    UFUNCTION(BlueprintCallable, Category = "Settings | Bool")
    bool SetDesiredValueBool(bool InBool);
    UFUNCTION(BlueprintCallable, Category = "Settings | Int")
    bool SetDesiredValueInt(int32 InInt);
    UFUNCTION(BlueprintCallable, Category = "Settings | Float")
    bool SetDesiredValueFloat(float InFloat);
    UFUNCTION(BlueprintCallable, Category = "Settings | Key")
    bool SetDesiredValueKeyBind(FRHKeyBind InKeyBind);

private:
    void SetDesiredValue(FSettingsUnion InValue);

// Get Option: These return the PreviewValue as the specified type.
public:
	UFUNCTION(BlueprintPure, Category = "Settings | Bool")
	FORCEINLINE bool GetPreviewValueBool() { return PreviewValue.HasSubtype<bool>() ? PreviewValue.GetSubtype<bool>() : false; }
	UFUNCTION(BlueprintPure, Category = "Settings | Int")
	FORCEINLINE int32 GetPreviewValueInt() { return PreviewValue.HasSubtype<int32>() ? PreviewValue.GetSubtype<int32>() : 0; }
	UFUNCTION(BlueprintPure, Category = "Settings | Float")
	FORCEINLINE float GetPreviewValueFloat() { return PreviewValue.HasSubtype<float>() ? PreviewValue.GetSubtype<float>() : 0.f; }

// Set Preview Options: Set the preview value.
public:
	UFUNCTION(BlueprintCallable, Category = "Settings | Bool")
	bool SetPreviewValueBool(bool InBool);
	UFUNCTION(BlueprintCallable, Category = "Settings | Int")
	bool SetPreviewValueInt(int32 InInt);
	UFUNCTION(BlueprintCallable, Category = "Settings | Float")
	bool SetPreviewValueFloat(float InFloat);

private:
	void SetPreviewValue(FSettingsUnion InValue);

	// Apply: This is NOT overridable. Implement ApplyType() for these to do anything.
public:
	UFUNCTION(BlueprintCallable, Category = "Apply Preview")
	void ApplyPreview();

	UFUNCTION(BlueprintCallable, Category = "Apply Preview")
	void ResetPreview();

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Bool")
	bool ApplyPreviewBoolValue(bool InBool);
	virtual bool ApplyPreviewBoolValue_Implementation(bool InBool) { return false; }
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Int")
	bool ApplyPreviewIntValue(int32 InInt);
	virtual bool ApplyPreviewIntValue_Implementation(int32 InInt) { return false; }
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Float")
	bool ApplyPreviewFloatValue(float InFloat);
	virtual bool ApplyPreviewFloatValue_Implementation(float InFloat) { return false; }

// Get Option: These return the Value as the specified type.
public:
    UFUNCTION(BlueprintPure, Category = "Settings | Bool")
    FORCEINLINE bool GetValueBool() { return Value.HasSubtype<bool>() ? Value.GetSubtype<bool>() : false; }
    UFUNCTION(BlueprintPure, Category = "Settings | Int")
    FORCEINLINE int32 GetValueInt() { return Value.HasSubtype<int32>() ? Value.GetSubtype<int32>() : 0; }
    UFUNCTION(BlueprintPure, Category = "Settings | Float")
    FORCEINLINE float GetValueFloat() { return Value.HasSubtype<float>() ? Value.GetSubtype<float>() : 0.f; }
    UFUNCTION(BlueprintPure, Category = "Settings | Key")
    FORCEINLINE FRHKeyBind GetValueKeyBind() { return Value.HasSubtype<FRHKeyGroup>() ? RHKeyGroupToRHKeyBind(Value.GetSubtype<FRHKeyGroup>()) : FRHKeyBind(); }

    UFUNCTION(BlueprintPure, Category = "Settings | Bool")
    FORCEINLINE bool GetDirtyValueBool() { return DirtyValue.HasSubtype<bool>() ? DirtyValue.GetSubtype<bool>() : false; }
    UFUNCTION(BlueprintPure, Category = "Settings | Int")
    FORCEINLINE int32 GetDirtyValueInt() { return DirtyValue.HasSubtype<int32>() ? DirtyValue.GetSubtype<int32>() : 0; }
    UFUNCTION(BlueprintPure, Category = "Settings | Float")
    FORCEINLINE float GetDirtyValueFloat() { return DirtyValue.HasSubtype<float>() ? DirtyValue.GetSubtype<float>() : 0.f; }
    UFUNCTION(BlueprintPure, Category = "Settings | Key")
    FORCEINLINE FRHKeyBind GetDirtyValueKeyBind() { return DirtyValue.HasSubtype<FRHKeyGroup>() ? RHKeyGroupToRHKeyBind(DirtyValue.GetSubtype<FRHKeyGroup>()) : FRHKeyBind(); }

// Apply: This is NOT overridable. Implement ApplyType() for these to do anything.
public:
    UFUNCTION(BlueprintCallable, Category = "Apply")
    void Apply();

protected:
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Bool")
    bool ApplyBoolValue(bool InBool);
    virtual bool ApplyBoolValue_Implementation(bool InBool) { return false; }
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Int")
    bool ApplyIntValue(int32 InInt);
    virtual bool ApplyIntValue_Implementation(int32 InInt) { return false; }
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Float")
    bool ApplyFloatValue(float InFloat);
    virtual bool ApplyFloatValue_Implementation(float InFloat) { return false; }
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Settings | Key")
    bool ApplyKeyBindValue(FRHKeyBind InKeyBind);
    virtual bool ApplyKeyBindValue_Implementation(FRHKeyBind InKeyBind) { return false; }

// Save: This is NOT overridable. Implement SaveType() for these to do anything.
public:
    UFUNCTION(BlueprintCallable, Category = "Save")
    void Save();

protected:
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Bool")
    bool SaveBoolValue(bool InBool);
    virtual bool SaveBoolValue_Implementation(bool InBool) { return false; }
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Int")
    bool SaveIntValue(int32 InInt);
    virtual bool SaveIntValue_Implementation(int32 InInt) { return false; }
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Float")
    bool SaveFloatValue(float InFloat);
    virtual bool SaveFloatValue_Implementation(float InFloat) { return false; }
    UFUNCTION(BlueprintNativeEvent, Category = "Settings | Key")
    bool SaveKeyBindValue(FRHKeyBind InKeyBind);
    virtual bool SaveKeyBindValue_Implementation(FRHKeyBind InKeyBind) { return false; }

private:
    FORCEINLINE void AttemptAutoApplyAndSave() { if (bIsAutoApplied) { Apply(); }  if (bIsAutoSaved) { Save(); } }

// Dirty: Check these to see if the setting info is in a dirty state.
public:
    UFUNCTION(BlueprintPure, Category = "Dirty")
    FORCEINLINE bool IsDirty() const { return !(Value == DirtyValue); }

// Revert: Check to see if the setting info can be reverted.
public:
    UFUNCTION(BlueprintCallable, Category = "Revert")
    void Revert();

public:
    UFUNCTION(BlueprintPure, Category = "Revert")
    FORCEINLINE bool CanRevert() const { return !(Value == RevertValue); }

// Callbacks: Call these when reacting to an apply of the setting we are interested in.
protected:
    UFUNCTION(BlueprintCallable, Category = "Settings | Bool")
    void OnValueBoolApplied(bool AppliedBool) { OnValueApplied(FSettingsUnion(AppliedBool)); }
    UFUNCTION(BlueprintCallable, Category = "Settings | Int")
    void OnValueIntApplied(int32 AppliedInt) { OnValueApplied(FSettingsUnion(AppliedInt)); }
    UFUNCTION(BlueprintCallable, Category = "Settings | Float")
    void OnValueFloatApplied(float AppliedFloat) { OnValueApplied(FSettingsUnion(AppliedFloat)); }
    UFUNCTION(BlueprintCallable, Category = "Settings | Key")
    void OnValueKeyBindApplied(FRHKeyBind AppliedKeyBind) { OnValueApplied(FSettingsUnion(RHKeyBindToRHKeyGroup(AppliedKeyBind))); }

    void OnValueApplied(FSettingsUnion AppliedValue);

// Callbacks: Call these when reacting to a save of the setting we are interested in.
protected:
    UFUNCTION(BlueprintCallable, Category = "Settings | Bool")
    void OnValueBoolSaved(bool SavedBool) { OnValueSaved(FSettingsUnion(SavedBool)); }
    UFUNCTION(BlueprintCallable, Category = "Settings | Int")
    void OnValueIntSaved(int32 SavedInt) { OnValueSaved(FSettingsUnion(SavedInt)); }
    UFUNCTION(BlueprintCallable, Category = "Settings | Float")
    void OnValueFloatSaved(float SavedFloat) { OnValueSaved(FSettingsUnion(SavedFloat)); }
    UFUNCTION(BlueprintCallable, Category = "Settings | Key")
    void OnValueKeyBindSaved(FRHKeyBind SavedKeyBind) { OnValueSaved(FSettingsUnion(RHKeyBindToRHKeyGroup(SavedKeyBind))); }

    void OnValueSaved(FSettingsUnion SavedValue);

public:
    UPROPERTY(BlueprintAssignable, Category = "Settings")
    FOnSettingValueChanged OnSettingValueChanged;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingPreviewChanged OnSettingPreviewChanged;

// Config: Set these based on how we want the object to react to dirtying of their setting.
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    bool bIsAutoApplied;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    bool bIsAutoSaved;

// Internal state: Here are the values we are representing for ourselves.
private:
    FSettingsUnion Value;
    FSettingsUnion DirtyValue;
    FSettingsUnion RevertValue;
	FSettingsUnion PreviewValue;
    bool bNeedsSave;

protected:
    UPROPERTY(EditdefaultsOnly, Category = "Settings")
    TArray<FText> TextOptions;

    // Get Options: Helpers for managing the text associated with index values.
public:
    UFUNCTION(BlueprintPure, Category = "Settings | Text")
    FORCEINLINE TArray<FText> const& GetTextOptions() const { return TextOptions; }
    UFUNCTION(BlueprintPure, Category = "Settings | Text")
    FORCEINLINE int32 GetNumTextOptions() const { return TextOptions.Num(); }
    UFUNCTION(BlueprintPure, Category = "Settings | Text")
    FORCEINLINE FText GetTextOption(int32 Index) const { return TextOptions.IsValidIndex(Index) ? TextOptions[Index] : FText(); }
    UFUNCTION(BlueprintCallable, Category = "Settings | Text")
    void UpdateTextOptions(const TArray<FText>& NewOptions);

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Settings | Text")
    FOnTextOptionsChanged OnTextOptionsChanged;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Settings")
    float MinValue;

    UPROPERTY(EditDefaultsOnly, Category = "Settings")
    float MaxValue;
	
    UPROPERTY(EditDefaultsOnly, Category = "Settings")
	bool bOverrideDisplayRange;

    UPROPERTY(EditDefaultsOnly, Category = "Settings", meta = (EditCondition = "bOverrideDisplayRange"))
    float MinDisplayValue;

    UPROPERTY(EditDefaultsOnly, Category = "Settings", meta = (EditCondition = "bOverrideDisplayRange"))
    float MaxDisplayValue;

    UPROPERTY(EditDefaultsOnly, Category = "Settings")
    float StepValue;

    UPROPERTY(EditDefaultsOnly, Category = "Settings")
    bool bRoundValue;

    UPROPERTY(EditDefaultsOnly, Category = "Settings", meta = (EditCondition = "bRoundValue"))
    float RoundToNearest;

    UPROPERTY(EditDefaultsOnly, Category = "Settings")
    bool bIsPercent;

    // Get MinMax: Helpers for managing the min and max values 
public:
    UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
    FORCEINLINE float GetMin() const { return MinValue; }
    UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
    FORCEINLINE float GetMax() const { return MaxValue; }
	UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
	FORCEINLINE float GetMinDisplay() const { return bOverrideDisplayRange ? MinDisplayValue : MinValue; }
	UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
	FORCEINLINE float GetMaxDisplay() const { return bOverrideDisplayRange ? MaxDisplayValue : MaxValue; }
    UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
    FORCEINLINE float GetStep() const { return StepValue; }
    UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
    FORCEINLINE bool GetRound() const { return bRoundValue; }
    UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
    FORCEINLINE float GetRoundToNearest() const { return RoundToNearest; }
    UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
    float RoundToNearestValueFloat(float ValueToRound) const;
    UFUNCTION(BlueprintPure, Category = "Settings | MinMax")
    FORCEINLINE bool GetIsPercent() const { return bIsPercent; }

// HUD: Use this to apply and save settings.
protected:
    UFUNCTION(BlueprintPure, Category = "HUD")
    class ARHHUDCommon* GetRHHUD() const;

private:
    static FRHKeyGroup RHKeyBindToRHKeyGroup(const FRHKeyBind& KeyBindToConvert);
    static FRHKeyBind RHKeyGroupToRHKeyBind(const FRHKeyGroup& KeyGroupToConvert);
};
