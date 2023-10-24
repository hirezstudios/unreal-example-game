// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "EnhancedPlayerInput.h"
#include "EnhancedInputComponent.h"
#if !UE_SERVER
#include "Framework/Application/IInputProcessor.h"
#endif
#include "Components/InputComponent.h"
#include "GameFramework/RHGameUserSettings.h"
#include "Containers/Union.h"
#include "RHPlayerInput.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKeyMappingsUpdated);

class URHAnalogStickFilter;

UENUM(BlueprintType)
enum RH_INPUT_STATE
{
	PIS_KEYMOUSE,
	PIS_GAMEPAD,
	PIS_TOUCH,
	PIS_UNKNOWN,
};

#if !UE_SERVER
class FRHInputPreProcessor : public IInputProcessor
{
public:
	FRHInputPreProcessor();

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent);
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) { return; };

	void BindPlayerInput(class URHPlayerInput* playerInput);
	bool IsInputStateAllowedInGame(RH_INPUT_STATE InputState) const;

	TWeakObjectPtr<class URHPlayerInput> PlayerInput;

	//allows us to persist input modes across map loads
	static RH_INPUT_STATE LastInputState;
};
#endif //!UE_SERVER

UENUM(BlueprintType)
enum class EKeyBindType : uint8
{
    ActionMapping UMETA(DisplayName = "Action Mapping"),
    AxisMapping UMETA(DisplayName = "Axis Mapping"),
};

UENUM(BlueprintType)
enum class EInputType : uint8
{
    KBM UMETA(DisplayName = "Keyboard / Mouse"),
    GP UMETA(DisplayName = "Gamepad"),
    Touch UMETA(DisplayName = "Touch"),
};

UENUM(BlueprintType)
enum class EInputActionType : uint8
{
	Press UMETA(DisplayName = "Press"),
	Hold UMETA(DisplayName = "Hold"),
	Repeat UMETA(DisplayName = "Repeat"),
};

USTRUCT(BlueprintType)
struct FRHInputActionNameTypePair
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	FName Name;
	UPROPERTY()
	EInputActionType Type;

	FRHInputActionNameTypePair() :
		Type(EInputActionType::Press)
	{ }
};

USTRUCT(BlueprintType)
struct FRHInputActionTiedNames
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	FName Press;
	UPROPERTY()
	FName Hold;
	UPROPERTY()
	FName Repeat;
};

USTRUCT(BlueprintType)
struct FRHCustomInputActionKey
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	int32 PropId;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FName KeyboardName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FName GamepadName;

	FRHCustomInputActionKey() :
		PropId(0)
	{ }
};

USTRUCT(BlueprintType)
struct FRHCustomInputAxisKey
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	int32 PropId;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FName KeyboardName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float KeyboardScale;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	FName GamepadName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float GamepadScale;

	FRHCustomInputAxisKey() :
		PropId(0),
		KeyboardScale(0.0f),
		GamepadScale(0.0f)
	{ }
};

USTRUCT(BlueprintType)
struct FRHInputActionKey
{
	GENERATED_USTRUCT_BODY()

	FRHInputActionKey()
		: Key(EKeys::Invalid)
		, Type(EInputActionType::Press)
	{}

	FRHInputActionKey(FKey InKey, EInputActionType InType)
		: Key(InKey)
		, Type(InType)
	{}

	bool operator==(const FRHInputActionKey& Other) const
	{
		return Key == Other.Key && Type == Other.Type;
	}

	UPROPERTY()
	FKey Key;

	UPROPERTY()
	EInputActionType Type;
};

// Workaround for Nested containers not being supported as UPROPERTY()
USTRUCT(BlueprintType)
struct FRHInputActionKeyMapping
{
	GENERATED_USTRUCT_BODY()

	FRHInputActionKeyMapping()
		: Mapping(FInputActionKeyMapping())
		, Type(EInputActionType::Press)
	{}

	FRHInputActionKeyMapping(FInputActionKeyMapping InMapping, EInputActionType InType)
		: Mapping(InMapping)
		, Type(InType)
	{}

	bool operator==(const FRHInputActionKeyMapping& Other) const
	{
		return Mapping == Other.Mapping && Type == Other.Type;
	}

	UPROPERTY()
	FInputActionKeyMapping Mapping;

	UPROPERTY()
	EInputActionType Type;
};

USTRUCT(BlueprintType)
struct FRHCustomInputActionKeyMappings
{
    GENERATED_USTRUCT_BODY()
    
    UPROPERTY()
    TArray<FRHInputActionKeyMapping> KBM_Mappings;

    UPROPERTY()
	TArray<FRHInputActionKeyMapping> GP_Mappings;

    UPROPERTY()
	TArray<FRHInputActionKeyMapping> Touch_Mappings;

	bool operator==(const FRHCustomInputActionKeyMappings& Other) const
	{
		return KBM_Mappings == Other.KBM_Mappings && GP_Mappings == Other.GP_Mappings && Touch_Mappings == Other.Touch_Mappings;
	}
	bool operator!=(const FRHCustomInputActionKeyMappings& Other) const
	{
		return !(*this == Other);
	}
};

// Workaround for Nested containers not being supported as UPROPERTY()
USTRUCT(BlueprintType)
struct FRHInputAxisKeyMappings
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TArray<FInputAxisKeyMapping> InputAxisKeyMappings;

	bool operator==(const FRHInputAxisKeyMappings& Other) const
	{
		return InputAxisKeyMappings == Other.InputAxisKeyMappings;
	}
};

USTRUCT(BlueprintType)
struct FRHCustomInputAxisKeyMappings
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TMap<float, FRHInputAxisKeyMappings> KBM_Mappings;

    UPROPERTY()
    TMap<float, FRHInputAxisKeyMappings> GP_Mappings;

    UPROPERTY()
    TMap<float, FRHInputAxisKeyMappings> Touch_Mappings;

	bool operator==(const FRHCustomInputAxisKeyMappings& Other) const
	{
		TArray<FRHInputAxisKeyMappings> OutKBMThis, OutKBMOther, OutGPThis, OutGPOther, OutTouchThis, OutTouchOther;
		KBM_Mappings.GenerateValueArray(OutKBMThis);
		Other.KBM_Mappings.GenerateValueArray(OutKBMOther);
		GP_Mappings.GenerateValueArray(OutGPThis);
		Other.GP_Mappings.GenerateValueArray(OutGPOther);
		Touch_Mappings.GenerateValueArray(OutTouchThis);
		Other.Touch_Mappings.GenerateValueArray(OutTouchOther);
		return OutKBMThis == OutKBMOther && OutGPThis == OutGPOther && OutTouchThis == OutTouchOther;
	}
	bool operator!=(const FRHCustomInputAxisKeyMappings& Other) const
	{
		return !(*this == Other);
	}
};

typedef void(URHPlayerInput::*FApplyInputSettingFunctionPtr)(const FName&);

UCLASS(config=Input, defaultconfig)
class RALLYHERESTART_API URHPlayerInputDefault : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(config)
	TArray<FSettingConfigPair> SettingsConfig;

	UPROPERTY(config)
	TArray<FRHInputActionNameTypePair> InputActionNameTypePairs;

	UPROPERTY(config)
	TArray<FRHInputActionTiedNames> InputActionTiedNames;

	UPROPERTY(config)
	TArray<FRHCustomInputActionKey> CustomInputActionKeys;

	UPROPERTY(config)
	TArray<FRHCustomInputAxisKey> CustomInputAxisKeys;
};

/**
 *
 */
UCLASS(config=Input)
class RALLYHERESTART_API URHPlayerInput : public UEnhancedPlayerInput
{
    GENERATED_BODY()
public:
    URHPlayerInput();
	~URHPlayerInput();
    virtual void PostInitProperties() override;

public:
	bool GetDefaultSettingAsBool(const FName& Name, bool& OutBool) const;
	bool GetDefaultSettingAsInt(const FName& Name, int32& OutInt) const;
	bool GetDefaultSettingAsFloat(const FName& Name, float& OutFloat) const;

	bool GetSettingAsBool(const FName& Name, bool& OutBool) const;
	bool GetSettingAsInt(const FName& Name, int32& OutInt) const;
	bool GetSettingAsFloat(const FName& Name, float& OutFloat) const;

	bool ApplySettingAsBool(const FName& Name, bool Value);
	bool ApplySettingAsInt(const FName& Name, int32 Value);
	bool ApplySettingAsFloat(const FName& Name, float Value);

	bool SaveSetting(const FName& Name);

	bool RevertSettingToDefault(const FName& Name);

private:
	void CallApplySettingFunction(const FName& Name);

public:
	// Custom Apply Setting Names
	static const FName MouseDoubleClickTime;
	static const FName MouseHoldTime;

	static const FName GamepadDoubleClickTime;
	static const FName GamepadHoldTime;

	static const FName MouseLookSensitivity;

	static const FName GamepadLookSensitivityX;
	static const FName GamepadLookSensitivityY;
	static const FName GamepadLeftDeadZone;
	static const FName GamepadRightDeadZone;

	static const FName GyroLookSensitivityX;
	static const FName GyroLookSensitivityY;
	static const FName GyroLookSensitivityADS;
	static const FName GyroLookSensitivityScope;

	static const FName GyroAimAssist;
	static const FName GyroRawInput;

	static const FName GamepadAccelerationBoost;
	static const FName GamepadMultiplierBoost;

	static const FName InvertX;
	static const FName InvertY;
	static const FName GyroInvertX;
	static const FName GyroInvertY;

	
	static const FName ContextualActionButtonMode;
	// End Custom Apply Setting Names

	void RevertSettingsToDefault();
	void ApplySavedSettings();
	void ApplySavedKeyMappings();

	// Custom Apply Functions
	void ApplyMouseLookSensitivity(const FName& Name);
	void ApplyGamepadAccelerationBoost(const FName& Name);
	void ApplyGamepadMultiplierBoost(const FName& Name);
	void ApplyInvertX(const FName& Name);
	void ApplyInvertY(const FName& Name);
	// End Custom Apply Functions

protected:
	//MASTER PROPERTY LIST
	UPROPERTY(config)
	TMap<FName, FString> SavedSettingsConfig;
	// Make this map from the SettingsConfig
	TMap<FName, FCachedSettingValue> DefaultSettingsConfig;
	// Load saved settings into this, update as settings change
	TMap<FName, FCachedSettingValue> AppliedSettingsConfig;
	TMap<FName, FApplyInputSettingFunctionPtr> ApplySettingFunctionMap;

public:
	// Getters that are kept as to not break existing code, use GetSettingAsType in the future
	bool GetInvertX() const;
	bool GetInvertY() const;
	FVector2D GetGamepadLookSensitivity() const;
	// End old getters

public:
	FORCEINLINE FVector2D GetGamepadLookAcceleration() { return GamepadLookAcceleration; }
    
    float ConvertGyroLookSensitivity(float value, bool isY = false) const;

	URHAnalogStickFilter* GetLeftAnalogStickFilter() { return LeftAnalogStickFilter; }
	URHAnalogStickFilter* GetRightAnalogStickFilter() { return RightAnalogStickFilter; }

protected:
	UPROPERTY(config)
	FVector2D GamepadLookAcceleration;

	UPROPERTY(config)
	float MinMouseSenseScaling;
	UPROPERTY(config)
	float MaxMouseSenseScaling;

	/*
	* Axis Key Binding custom settings
	*/

public:
	void SaveKeyMappings();
    // Gets rid of the current default mappings for custom mappings
    void RemoveCustomDefaultKeyMappings();
    // converts the default action mappings to our custom type, so we can manage them easier
    void SetCustomKeyMappingsToDefault();
	// applies all of the current custom axis and action mappings
	void ApplyCustomMappings();
	// removes any custom mappings that are no longer customizable
	void RemoveStaleCustomBindings();

	void GetCustomActionKeyMappingNames(TArray<FName>& OutNames) const;
	void GetCustomAxisKeyMappingNames(TArray<FName>& OutNames) const;

    void GetCustomActionMappingForDefaultMapping(const FName& ActionMappingName, FRHCustomInputActionKeyMappings& OutCustomInputActionKeyMappings) const;
    void GetCustomAxisMappingForDefaultMapping(const FName& AxisMappingName, FRHCustomInputAxisKeyMappings& OutCustomInputAxisKeyMappings) const;
    
private:
    TArray<FRHInputActionKeyMapping>* GetCustomInputActionKeyMappingsFor(const FName& Name, EInputType InputType);
    void GetCustomInputActionKeyMappingsCopyFor(const FName& Name, EInputType InputType, TArray<FRHInputActionKeyMapping>& OutInputAxisKeyMapping) const;
    TArray<FInputAxisKeyMapping>* GetCustomInputAxisKeyMappingsFor(const FName& Name, EInputType InputType, float Scale);
    void GetCustomInputAxisKeyMappingsCopyFor(const FName& Name, EInputType InputType, float Scale, TArray<FInputAxisKeyMapping>& OutInputAxisKeyMapping) const;

    void GetCustomInputAxisKeyMappingsFor(const FName& Name, EInputType InputType, TMap<float, TArray<FInputAxisKeyMapping>*>& OutCustomInputAxisKeyMappingsList);
	void GetCustomInputAxisKeyMappingsCopyFor(const FName& Name, EInputType InputType, TMap<float, TArray<FInputAxisKeyMapping>>& OutCustomInputAxisKeyMappingsList) const;

public:
	void GetDefaultInputActionKeys(const FName& Name, EInputType InputType, TArray<FRHInputActionKey>& OutKeys) const;
    void GetCustomInputActionKeys(const FName& Name, EInputType InputType, TArray<FRHInputActionKey>& OutKeys) const;
	void GetDefaultInputAxisKeys(const FName& Name, EInputType InputType, float Scale, TArray<FKey>& OutKeys) const;
    void GetCustomInputAxisKeys(const FName& Name, EInputType InputType, float Scale, TArray<FKey>& OutKeys) const;

	FName GetCustomInputActionWithKeys(const TArray<FRHInputActionKey>& Keys, EInputType InputType) const;
	TPair<FName, float> GetCustomInputAxisWithKeys(const TArray<FKey>& Keys, EInputType InputType) const;

	void SetCustomInputActionKeyMapping(const FName& Name, const TArray<FRHInputActionKey>& Keys, EInputType InputType, TArray<FName>& OutModifiedKeybindNames);
    void SetCustomInputActionKeyMapping_Internal(const FName& Name, const TArray<FRHInputActionKey>& Keys, EInputType InputType, TArray<FName>& OutModifiedKeybindNames);
    void SetCustomInputAxisKeyMapping(const FName& Name, const TArray<FKey>& Keys, EInputType InputType, float Scale, TArray<FName>& OutModifiedKeybindNames);

private:
    void ResolveCustomInputActionKeyMappingConflicts(const TArray<FRHInputActionKey>& ConflictingKeys, const TArray<FRHInputActionKey>& ReplacementKeys, EInputType InputType, TArray<FName>& OutModifiedKeybindNames);
    void ResolveCustomInputAxisKeyMappingConflicts(const TArray<FKey>& ConflictingKeys, const TArray<FKey>& ReplacementKeys, EInputType InputType, TArray<FName>& OutModifiedKeybindNames);

public:
    void GetSoloInputActionKeyMappings(const TArray<FName>& MappingNames, TArray<TPair<FName, FRHInputActionKey>>& OutSoloMappings);
    void GetChordInputActionKeyMappings(const TArray<FName>& MappingNames, TArray<TPair<FName, TArray<FRHInputActionKey>>>& OutChordMappings);

    UPROPERTY(BlueprintAssignable, Category = "KeyMappings")
    FOnKeyMappingsUpdated OnKeyMappingsUpdated;
	
protected:
    UPROPERTY(config)
	TMap<FName, FRHCustomInputActionKeyMappings> CustomActionKeyMappings;
	TMap<FName, FRHCustomInputActionKeyMappings> AppliedActionKeyMappings;

    UPROPERTY(config)
	TMap<FName, FRHCustomInputAxisKeyMappings> CustomAxisKeyMappings;
	TMap<FName, FRHCustomInputAxisKeyMappings> AppliedAxisKeyMappings;

public:
	UFUNCTION(BlueprintPure, Category = "Input")
	FORCEINLINE bool DoesInputTypeSupportChords(EInputType InputType) const { return InputType == EInputType::GP; }

public:
	virtual bool InputKey(const FInputKeyParams& Params);
	virtual bool InputTouch(uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex);
	virtual bool InputMotion(const FVector& Tilt, const FVector& RotationRate, const FVector& Gravity, const FVector& Acceleration);
	virtual bool InputGesture(const FKey Gesture, const EInputEvent Event, const float Value);
	virtual void ProcessInputStack(const TArray<UInputComponent*>& InputComponentStack, const float DeltaTime, const bool bGamePaused);

	DECLARE_EVENT_OneParam(URHPlayerInput, FInputStateChangedEvent, RH_INPUT_STATE);
	virtual FInputStateChangedEvent& OnInputStateChanged() { return InputStateChangedEvent; }

	FORCEINLINE bool IsUsingGamepad() const { return InputState == PIS_GAMEPAD; }
	FORCEINLINE bool IsUsingKeyboard() const { return InputState == PIS_KEYMOUSE; }
	FORCEINLINE RH_INPUT_STATE GetInputState() const { return InputState; }
	void PreProcessInputUpdate(RH_INPUT_STATE NewState, bool bAxis, float Delta) { UpdateInputState(NewState, bAxis, Delta); };

	static FKey GetGamepadConfirmButton();
	static FKey GetGamepadCancelButton();

    virtual void SaveStickFilterConfig();

protected:
	UPROPERTY(Config)
	TSubclassOf<URHAnalogStickFilter> LeftAnalogStickFilterClass;
	UPROPERTY(Config)
	TSubclassOf<URHAnalogStickFilter> RightAnalogStickFilterClass;

	RH_INPUT_STATE InputState;
    FInputStateChangedEvent InputStateChangedEvent;

    UPROPERTY(config)
    float KeyMouseSwitchDelta;
    float CurrentDelta;

    UPROPERTY(config)
    float GamepadSwitchDelta;
    float PrevGamepadDelta;

	bool CanChangeToInputState(RH_INPUT_STATE NewState, bool bAxis, float Delta);
	void UpdateInputState(RH_INPUT_STATE NewState, bool bAxis, float Delta);

	virtual void NotifyInputStateChanged();	//$$ LDP - New virtual method to allow hooks of derived classes before broadcasting a change

private:
	UPROPERTY(Transient)
	URHAnalogStickFilter* LeftAnalogStickFilter;
	bool bHasProcessedLeftStick;
	FVector2D ProcessedLeftStick;
	UPROPERTY(Transient)
	URHAnalogStickFilter* RightAnalogStickFilter;
	bool bHasProcessedRightStick;
	FVector2D ProcessedRightStick;

	virtual float MassageAxisInput(FKey Key, float RawValue) override;

#if !UE_SERVER
	TSharedPtr<FRHInputPreProcessor> InputDetector;
#endif //!UE_SERVER
};
