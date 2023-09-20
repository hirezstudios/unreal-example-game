#pragma once

#include "GameFramework/GameUserSettings.h"
#include "GameFramework/SaveGame.h"
#include "RH_Properties.h"
#include "RHGameUserSettings.generated.h"

UENUM(BlueprintType)
enum class EADSMode : uint8
{
	Hold,
	Toggle,
	Both,
};

//When changing this enum, be sure to update URHExampleGameUserSettings::SetGyroMode(int32) and URHSettingsDataFactory::InitSettingsForPlayer()
UENUM(BlueprintType)
enum class EGyroMode : uint8
{
    None,
    AimOnly,
    Always
};

UENUM(BlueprintType)
enum class EGamepadIcons : uint8
{
    XboxOne,
    PlayStation4,
    NintendoSwitch
};

UENUM(BlueprintType)
enum class ECrosshairSize : uint8
{
	Standard,
	Medium,
	Large,
};

//Different options for what muting a player does
UENUM(BlueprintType)
enum class EMuteMode : uint8
{
	VoicechatOnly,
	VoicechatAndQuips,
	VoicechatAndCommunications,
	VoicechatQuipsCommunications,
};

struct FCachedSettingValue
{
	FString StringValue;
	int32 IntValue;
	float FloatValue;
	bool BoolValue;

	FCachedSettingValue() : StringValue(TEXT("")), IntValue(0), FloatValue(0.f), BoolValue(false)
	{}
	FCachedSettingValue(const FString& InValue);
	FCachedSettingValue(int32 InValue);
	FCachedSettingValue(float InValue);
	FCachedSettingValue(bool InValue);
};

UCLASS()
class RALLYHERESTART_API URHSettingsSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
	TMap<FName, FString> SavedSettingsConfig;

	UPROPERTY()
	FString SavedDisplayLanguage;

	UPROPERTY()
	TSet<FName> SavedLocalActions;

	UPROPERTY()
	FString SavedSelectedRegion;

	UPROPERTY()
	int32 LastWhatsNewVersion;

	UPROPERTY()
	TArray<FRH_LootId> SavedTransientOrderIds;

	UPROPERTY()
	TSet<FName> SavedViewedNewsPanelIds;

	UPROPERTY()
	TSet<FRH_LootId> SavedRecentlySeenStoreItemLootIds;

	UPROPERTY()
	TSet<FRH_ItemId> SavedSeenAcquiredItemIds;
};

USTRUCT()
struct RALLYHERESTART_API FSettingConfigPair
{
	GENERATED_BODY()
public:
	FSettingConfigPair() {}
	FSettingConfigPair(const FName& InName, const FString& InValue)
		: Name(InName)
		, Value(InValue)
	{}

	UPROPERTY()
		FName Name;
	UPROPERTY()
		FString Value;

	// We don't care about the value being equal, use this for adding/removing
	bool operator==(const FSettingConfigPair& Other) const
	{
		return this->Name == Other.Name;
	}
};

UCLASS(config = GameUserSettings, defaultconfig)
class RALLYHERESTART_API URHGameUserSettingsDefault : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(config)
	TArray<FSettingConfigPair> SettingsConfig;
};

typedef void(URHGameUserSettings::*FApplySettingFunctionPtr)(const FName&);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGamepadIconSetSettingsApplied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalSettingSaved, FName, SettingName);

UCLASS(BlueprintType)
class RALLYHERESTART_API URHGameUserSettings : public UGameUserSettings
{
    GENERATED_BODY()

public:
	URHGameUserSettings(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE int32 GetSettingsVersionMajor() const { return SettingsVersionMajor; }
	void SetSettingsVersionMajor(int32 VersionMajor) { SettingsVersionMajor = VersionMajor; }
	FORCEINLINE int32 GetSettingsVersionMinor() const { return SettingsVersionMinor; }
	void SetSettingsVersionMinor(int32 VersionMinor) { SettingsVersionMinor = VersionMinor; }

    /** Applies all current user settings to the game and saves to permanent storage (e.g. file), optionally checking for command line overrides. */
	void RevertSettingsToDefault();
	void ApplySavedSettings();
    virtual void LoadSettings(bool bForceReload = false) override;
	virtual void SaveSettings() override;

protected:
	bool bIsLoading;

public:
    virtual bool LoadSaveGameConfig();
    virtual bool SaveSaveGameConfig();

	bool GetDefaultSettingAsBool(const FName& Name, bool& OutBool) const;
	bool GetDefaultSettingAsInt(const FName& Name, int32& OutInt) const;
	bool GetDefaultSettingAsFloat(const FName& Name, float& OutFloat) const;

	bool GetSettingAsBool(const FName& Name, bool& OutBool) const;
	bool GetSettingAsInt(const FName& Name, int32& OutInt) const;
	bool GetSettingAsFloat(const FName& Name, float& OutFloat) const;

	template<typename TEnumType>
	bool GetSettingAsEnum(FName Name, TEnumType& OutEnumValue) const
	{
		int32 ValueAsInt;
		if (!GetSettingAsInt(Name, ValueAsInt))
		{
			return false;
		}

		const UEnum* Enum = StaticEnum<TEnumType>();
		if (Enum && Enum->IsValidEnumValue(ValueAsInt))
		{
			OutEnumValue = (TEnumType)ValueAsInt;
			return true;
		}
		
		return false;
	}

	bool ApplySettingAsBool(const FName& Name, bool Value);
	bool ApplySettingAsInt(const FName& Name, int32 Value);
	bool ApplySettingAsFloat(const FName& Name, float Value);

	bool SaveSetting(const FName& Name);

	// Mark a generic action name as saved locally
	UFUNCTION(BlueprintCallable)
	void SaveLocalAction(const FName& Name);

	UFUNCTION(BlueprintPure)
	bool IsLocalActionSaved(const FName& Name) const { return SavedLocalActions.Contains(Name); }

	bool RevertSettingToDefault(const FName& Name);

private:
	void CallApplySettingFunction(const FName& Name);

public:
	// Custom Apply Setting Names
	static const FName MasterSoundVolume;
	static const FName MusicSoundVolume;
	static const FName SFXSoundVolume;

	// Music sub-settings
	static const FName MusicCharacterSelectVolume;
	static const FName MusicMainMenuVolume;
	static const FName MusicRoundStartVolume;
	static const FName MusicMatchEndVolume;
	static const FName MusicRoundEndVolume;
	static const FName MusicTimeRemainingVolume;
	static const FName MusicCharacterVolume;

	// SFX sub-settings
	static const FName SFXWepWrapVolume;

	static const FName VoiceSoundVolume;
	static const FName SpatialSoundEnabled;
	static const FName SafeFrameScale;
	static const FName VSyncEnabled;
	static const FName Brightness;
	static const FName BrightnessHandheld;
	static const FName AimMode;
	static const FName RadialWheelMode;
	static const FName GamepadRadialWheelMode;
	static const FName QuickCastEnabled;
	static const FName DamageNumbersEnabled;
	static const FName DamageNumbersStacked;
	static const FName EnemyAimedAtOutlined;
	static const FName CrosshairSize;
	static const FName GyroMode;
	static const FName MuteMode;
	static const FName QuipsEnabled;
	static const FName CommunicationsEnabled;
	static const FName TextChatEnabled;
	static const FName VoiceChatEnabled;
	static const FName PushToTalkEnabled;
	static const FName CrossPlayEnabled;
	static const FName InputCrossPlayEnabled;
	static const FName MotionBlurEnabled;
	static const FName ScreenMode;
	static const FName ForceFeedbackEnabled;
	static const FName ColorCorrection;
	static const FName GlobalQuality;
	static const FName ViewDistanceQuality;
	static const FName AntiAliasingQuality;
	static const FName ShadowQuality;
	static const FName PostProcessQuality;
	static const FName TextureQuality;
	static const FName EffectsQuality;
	static const FName FoliageQuality;
	static const FName PingEnabled;
	static const FName PacketLossEnabled;
	static const FName FPSEnabled;
	static const FName MobilePerformanceMode;
	static const FName TouchFireMode;
	static const FName TouchModeAutoVaultEnabled;
	// End Custom Apply Setting Names

	// Custom Apply Functions
	void ApplySoundVolume(const FName& Name);
	void ApplyVoiceSoundVolume(const FName& Name);
	void ApplySpatialSoundEnabled(const FName& Name);
	void ApplySafeFrameScale(const FName& Name);
	void ApplyVSyncEnabled(const FName& Name);
	void ApplyBrightness(const FName& Name);
	void ApplyBrightnessHandheld(const FName& Name);
	void ApplyVoiceChatEnabled(const FName& Name);
	void ApplyPushToTalk(const FName& Name);
	void ApplyCrossPlay(const FName& Name);
	void ApplyInputCrossPlay(const FName& Name);
	void ApplyMotionBlur(const FName& Name);
	void ApplyScreenMode(const FName& Name);
	void ApplyForceFeedback(const FName& Name);
	void ApplyColorCorrection(const FName& Name);
	void ApplyGlobalQuality(const FName& Name);
	void ApplyQuality(const FName& Name);
	void ApplyTelemetrySettings(const FName& Name);
	// End Custom Apply Functions

// Misc. Helper functions
public:
// These Getters exist as helpers so we don't have to change preexisting code. Use GetSettingAsType for getting new settings.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetSafeFrameScale() const;
	bool SetSafeFrameScaleToSystemOverride(); //returns true if a change occurred.
	int32 GetADSMode() const;
	UFUNCTION(BlueprintPure)
	bool GetUseQuickCast() const;
	bool GetDamageNumbersDisplayEnabled() const;
	bool GetDamageNumbersDisplayStacking() const;
	bool GetEnemyAimedAtOutlined() const;
	UFUNCTION(BlueprintPure)
	ECrosshairSize GetCrosshairSize() const;
	EGyroMode GetGyroMode() const;
	UFUNCTION(BlueprintPure)
	EMuteMode GetMuteMode() const;
	UFUNCTION(BlueprintPure)
	bool GetQuipsEnabled() const;
	UFUNCTION(BlueprintPure)
	bool GetCommunicationsEnabled() const;
	UFUNCTION(BlueprintPure)
	bool GetTextChatEnabled() const;
	UFUNCTION(BlueprintPure)
	bool GetVoiceChatEnabled() const;
	UFUNCTION(BlueprintPure)
	EColorVisionDeficiency GetColorCorrection() const;
	bool GetPushToTalk() const;
	bool GetPlatformCrossPlay() const;
	bool GetInputCrossPlay() const;
	bool GetUseForceFeedback() const;
	int32 GetScreenMode() const;
// End old getters

	EGamepadIcons GetGamepadIconSet() const;
	//Never expose this directly to Blueprint!  Setting should be done via normal Settings Data Factory means
	void SetGamepadIconSet(EGamepadIcons IconSet);

// Settings that don't use the generic system
	FORCEINLINE void SaveScreenResolution() { SavedScreenResolution = GetScreenResolution(); }
	void RevertScreenResolution();

	FORCEINLINE FString GetCurrentLanguage() const { return CurrentDisplayLanguage; }
	void SetCurrentLanguage(FString Language);
	FORCEINLINE void SaveCurrentLanguage() { SavedDisplayLanguage = CurrentDisplayLanguage; }

	FORCEINLINE FString GetSelectedRegion() const { return SavedSelectedRegion; }
	FORCEINLINE void SaveSelectedRegion(const FString& NewRegion) { SavedSelectedRegion = NewRegion; }

	FORCEINLINE int32 GetWhatsNewVersion() const { return LastWhatsNewVersion; }
	FORCEINLINE void SaveWhatsNewVersion(int32 NewVersion) { LastWhatsNewVersion = NewVersion; }

	FORCEINLINE const TArray<FRH_LootId>& GetSavedTransientOrderIds() const { return SavedTransientOrderIds; }
	bool SaveTransientOrderId(const FRH_LootId& LootId);

	FORCEINLINE const TSet<FName>& GetSavedViewedNewsPanelIds() const { return SavedViewedNewsPanelIds; }
	bool SaveViewedNewPanelId(FName UniqueName);

	FORCEINLINE const TSet<FRH_LootId>& GetSavedRecentlySeenStoreItemLootIds() const { return SavedRecentlySeenStoreItemLootIds; }
	void SaveRecentlySeenStoreItemLootIds(const TSet<FRH_LootId>& LootIds);
	bool SaveRecentlySeenStoreItemLootId(const FRH_LootId& LootId);

	FORCEINLINE const TSet<FRH_ItemId>& GetSavedSeenAcquiredItemIds() const { return SavedSeenAcquiredItemIds; }
	void SaveSeenAcquiredItemIds(const TSet<FRH_ItemId>& ItemIds);
	bool SaveSeenAcquiredItemId(const FRH_ItemId& ItemId);

public:
	// Lets local controller know when telemetry settings have changed
	DECLARE_EVENT(URHExampleGameUserSettings, FOnTelemetrySettingsAppliedNative);
	FOnTelemetrySettingsAppliedNative& OnTelemetrySettingsApplied() { return OnTelemetrySettingsAppliedNativeDel; }
protected:
	FOnTelemetrySettingsAppliedNative  OnTelemetrySettingsAppliedNativeDel;

public:
	DECLARE_EVENT(URHExampleGameUserSettings, FOnSettingsSavedNative);
	FOnSettingsSavedNative& OnSettingsSaved() { return OnSettingsSavedNativeDel; }
protected:
	FOnSettingsSavedNative  OnSettingsSavedNativeDel;

protected:
	UPROPERTY(config)
	int32 SettingsVersionMajor;

	UPROPERTY(config)
	int32 SettingsVersionMinor;

	UPROPERTY(config)
	EGamepadIcons GamepadIconSet;

	UPROPERTY(config)
	FIntPoint DefaultScreenResolution;
	UPROPERTY(config)
	FIntPoint SavedScreenResolution;
	
	bool bSettingGlobalQuality;
	Scalability::FQualityLevels AutoQualitySettings;

	FString CurrentDisplayLanguage;
	UPROPERTY(config)
	FString SavedDisplayLanguage;

	UPROPERTY(config)
	FString SavedSelectedRegion;

	UPROPERTY(config)
	int32 LastWhatsNewVersion;

	UPROPERTY(config)
	TArray<FRH_LootId> SavedTransientOrderIds;

	UPROPERTY(config)
	TSet<FName> SavedViewedNewsPanelIds;

	UPROPERTY(config)
	TSet<FRH_LootId> SavedRecentlySeenStoreItemLootIds;

	UPROPERTY(config)
	TSet<FRH_ItemId> SavedSeenAcquiredItemIds;

	//MASTER PROPERTY LIST
	UPROPERTY(config)
	TMap<FName, FString> SavedSettingsConfig;
	// Set containing names of generic actions that have been performed locally
	UPROPERTY(config)
	TSet<FName> SavedLocalActions;

	// Make this map from the SettingsConfig
	TMap<FName, FCachedSettingValue> DefaultSettingsConfig;

	// Load saved settings into this, update as settings change
	TMap<FName, FCachedSettingValue> AppliedSettingsConfig;
	TMap<FName, FApplySettingFunctionPtr> ApplySettingFunctionMap;

public:
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnGamepadIconSetSettingsApplied OnGamepadIconSetSettingsApplied;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnLocalSettingSaved OnLocalSettingSaved;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnLocalSettingSaved OnSettingApplied;
};
