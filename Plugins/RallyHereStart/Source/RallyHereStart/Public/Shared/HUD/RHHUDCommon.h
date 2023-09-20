#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UObject/Object.h"

#include "GameFramework/RHPlayerInput.h"
#include "InputAction.h"
#include "RHUISoundTheme.h"
#include "Managers/RHInputManager.h"
#include "Managers/RHViewManager.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "Templates/IsInvocable.h"
#include "Templates/EnableIf.h"
#include "Templates/PointerIsConvertibleFromTo.h"
#include "RH_PlayerInfoSubsystem.h"

#include "RHHUDCommon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputStateChange, RH_INPUT_STATE, InputState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerDataUpdated, URH_PlayerInfo*, PlayerInfo);
DECLARE_MULTICAST_DELEGATE_OneParam(FPlayerInfoReceived, URH_PlayerInfo* /*PlayerInfo*/);

USTRUCT(BlueprintType)
struct FColorPaletteInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Palette Info")
	FLinearColor LinearColor;

	FColorPaletteInfo() :
		LinearColor(FLinearColor(0,0,0,0))
	{ }
};

USTRUCT(BlueprintType)
struct FFontPaletteInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Font Palette Info")
	FSlateFontInfo FontInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreferredRegionUpdated);

UCLASS(config = Game)
class RALLYHERESTART_API ARHHUDCommon : public AHUD
{
    GENERATED_BODY()

public:
	ARHHUDCommon(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "Data Factories", meta = (DisplayName = "Get Login Data Factory"))
    virtual class URHLoginDataFactory* GetLoginDataFactory() const { return LoginDataFactory; };

    // Returns the Store Item Helper to be able to access store items
    UFUNCTION(BlueprintCallable, Category = "Data Factories")
    virtual class URHStoreItemHelper*    GetItemHelper() const;

    // Returns the UI Session Manager for session based state
    UFUNCTION(BlueprintCallable, Category = "Data Factories")
    virtual class URHUISessionManager*   GetUISessionManager() const;

    // Returns the Order Manager to be able to hook into any player orders
    UFUNCTION(BlueprintCallable, Category = "Managers")
    class URHOrderManager*    GetOrderManager() const;

	UFUNCTION(BlueprintCallable, Category = "Managers")
	class URHInputManager* GetInputManager() { return InputManager; }

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Managers")
	class URHPopupManager* GetPopupManager();

    UFUNCTION(BlueprintCallable, Category = "Data Factories", meta = (DisplayName = "Get Loadout Data Factory"))
    virtual class URHLoadoutDataFactory*         GetLoadoutDataFactory() const;

    UFUNCTION(BlueprintCallable, Category = "Data Factories")
    virtual class URHSettingsDataFactory* GetSettingsDataFactory() const { return SettingsFactory; };

	UFUNCTION(BlueprintCallable, Category = "Managers")
	virtual class URHPartyManager* GetPartyManager() const { return PartyManager; };

	UFUNCTION(BlueprintCallable, Category = "Managers")
	void SetViewManager(class URHViewManager* InViewManager) { ViewManager = InViewManager; };

	UFUNCTION(BlueprintPure, Category = "Managers")
	class URHViewManager* GetViewManager() const { return ViewManager; };

    UFUNCTION(BlueprintImplementableEvent, BlueprintPure, Category = "Gamepad | Prompts")
    TArray<class UPanelWidget*> GetFocusableWidgetContainers();

    UFUNCTION(BlueprintPure, Category = "Crossplay")
    bool IsSamePlatformAsLocalPlayer(const FGuid& PlayerId) const;

    UFUNCTION(BlueprintPure, Category = "Crossplay")
    bool ShouldShowCrossplayIconForPlayer(const FGuid& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Crossplay")
	bool ShouldShowCrossplayIconForPlayerState(class ARHPlayerState* PlayerState) const;

    UFUNCTION(BlueprintCallable, Category = "Preferred Region Id")
    void SetPreferredRegionId(const FString& RegionId);

    UFUNCTION(BlueprintPure, Category = "Preferred Region Id")
    void GetRegionList(TMap<FString, FText>& OutRegionIdToNameMap) const;

    UPROPERTY(BlueprintAssignable, Category = "Preferred Region Id")
    FOnPreferredRegionUpdated OnPreferredRegionUpdated;

    UFUNCTION(BlueprintCallable, Category = "Popup Manager")
    void ShowErrorPopup(FText ErrorMsg);

    UFUNCTION(BlueprintCallable, Category = "RH Hud Common")
    void LogErrorMessage(FText ErrorMsg);

	UFUNCTION()
	void OnInvalidVoucherOrder(URHStoreItem* StoreItem);

    UFUNCTION(BlueprintCallable, Category = "RH HUD Common")
    void UIX_ReportServer();

    UFUNCTION(BlueprintCallable, Category = "RH HUD Common")
    void PrintToLog(FText InText);

    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "RH HUD Common")
    void EvaluateFocus();

	void ProcessSoundEvent(FName SoundEventName);

	UFUNCTION(BlueprintCallable, Category = "RH HUD Common | Navigation")
	void SetNavigationEnabled(bool Enabled);

	UFUNCTION(BlueprintCallable, Category = "RH HUD Common | Popups")
	void DisplayGenericPopup(const FString& sTitle, const FString& sDesc);

	UFUNCTION(BlueprintCallable, Category = "RH HUD Common | Popups")
	void DisplayGenericError(const FString& sDesc);

	UFUNCTION(BlueprintPure, Category = "RH HUD Common")
    class APlayerController* GetPlayerControllerOwner() { return GetOwningPlayerController(); }

    UFUNCTION(BlueprintImplementableEvent, Category = "RH HUD Common | Features")
    void SetUseNewUIFeatures(bool UseNewFeatures);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "RH HUD Common | Navigation")
    bool OnNavigateBack();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RH HUD Common | Navigation")
    void SetUIFocus();
    virtual void SetUIFocus_Implementation();

    UFUNCTION(BlueprintCallable, Category = "RH HUD Common | Navigation")
    virtual void OnQuit();

	UFUNCTION(BlueprintCallable, Category = "RH HUD Common | Navigation")
	void OnLogout();

    UFUNCTION()
    void InputStateChangePassthrough(RH_INPUT_STATE inputState);

	UFUNCTION()
	void OnRegionsUpdated(URH_MatchmakingBrowserCache* MatchingBrowserCache);

	UFUNCTION(BlueprintPure, Category = "Preferred Region Id")
	virtual bool GetPreferredRegionId(FString& OutRegionId) const;

    UFUNCTION(BlueprintCallable, Category = "RH HUD Common | Player Input")
    RH_INPUT_STATE GetCurrentInputState();

	UFUNCTION()
	FPlatformUserId GetPlatformUserId() const;
	UFUNCTION()
	FUniqueNetIdWrapper GetOSSUniqueId() const;
	UFUNCTION()
	class URH_LocalPlayerSubsystem* GetLocalPlayerSubsystem() const;
	UFUNCTION()
	class URH_GameInstanceSubsystem* GetGameInstanceSubsystem() const;

	UFUNCTION()
	URH_PlayerInfoSubsystem* GetPlayerInfoSubsystem() const;

    UFUNCTION()
    URH_PlayerInfo* GetOrCreatePlayerInfo(const FGuid& PlayerUuid);
	URH_PlayerPlatformInfo* GetOrCreatePlayerPlatformInfo(const FRH_PlayerPlatformId& Identity);
	URH_PlayerInfo* GetPlayerInfo(const FGuid& PlayerUuid);
	URH_PlayerPlatformInfo* GetPlayerPlatformInfo(const FRH_PlayerPlatformId& Identity);

	URH_PlayerInfo* GetLocalPlayerInfo();
	FGuid GetLocalPlayerUuid();
	FRH_PlayerPlatformId GetLocalPlayerPlatformId();

    UFUNCTION(BlueprintPure, Category = "RH HUD Common | Player", meta=(DisplayName="IsPlatformCrossplayEnabled"))
    bool IsCrossplayEnabled() const;
	FORCEINLINE bool IsPlatformCrossplayEnabled() const { return IsCrossplayEnabled(); }

    UPROPERTY(BlueprintAssignable, Category = "RH HUD Common | Input")
    FInputStateChange OnInputStateChanged;

    UFUNCTION(BlueprintPure, Category = "RH HUD Common")
    virtual bool IsLobbyHUD() const { return false; }

protected:
    virtual void InstanceDataFactories();
    virtual void InitializeDataFactories();
	virtual void InitializeSoundTheme();
    virtual void UninitializeDataFactories();

	void CreateInputManager();

	UFUNCTION()
	virtual void OnConfirmQuit();

	UFUNCTION()
	void OnConfirmLogout();

    UFUNCTION()
    void ConfirmReportServer();

    UPROPERTY(Transient)
    class URHLoginDataFactory* LoginDataFactory;

    UPROPERTY(Transient)
    class URHSettingsDataFactory* SettingsFactory;

	UPROPERTY(Transient)
	class URHPartyManager* PartyManager;

	UPROPERTY(Transient)
	class URHInputManager* InputManager;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "ViewManager")
	class URHViewManager* ViewManager;

	UPROPERTY(EditDefaultsOnly, Category = "InputManager")
	TSubclassOf<URHInputManager> InputManagerClass;

private:
	TWeakObjectPtr<URHPlayerInput> PlayerInput;
    /*
    * Text Chat controls pass through
    */
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Safe Frame")
	void ApplySafeFrameScale(float SafeFrameScale);

/**
* Console
*/
protected:
	UFUNCTION()
	void HandleControllerDisconnect();

/**
* Color Palette
*/
public:
	UFUNCTION(BlueprintPure, Category = "Color Palette")
	bool GetColor(FName ColorName, FLinearColor& ReturnColor) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Color Palette")
	UDataTable* ColorPaletteDT;

/**
* Font Palette
*/
public:
	UFUNCTION(BlueprintPure, Category = "Font Palette")
	bool GetFont(FName FontName, FSlateFontInfo& ReturnFont) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Font Palette")
	UDataTable* FontPaletteDT;

	UPROPERTY(Transient)
	class URHUISoundTheme* SoundTheme;

public:
	UFUNCTION(BlueprintPure, Category = "Voice Chat")
	bool IsMuted(const FGuid& PlayerUuid);

	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
	bool MutePlayer(const FGuid& PlayerUuid, bool Mute);

	// HUD Drawn hooks
public:
	DECLARE_EVENT(ARHHUDCommon, FHUDDrawnEvent)
	FHUDDrawnEvent& OnHUDDrawn() { return HUDDrawn; }
	virtual void DrawHUD() override;

private:
	bool HasHUDBeenDrawn;
	FHUDDrawnEvent HUDDrawn;

	/*
	* Enhanced Player Input Actions
	*/
public:
	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetAcceptPressedAction() { return AcceptPressedAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetAcceptReleasedAction() { return AcceptReleasedAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetCancelPressedAction() { return CancelPressedAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetCancelReleasedAction() { return CancelReleasedAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetHoldToConfirmAction() { return HoldToConfirmAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetHoldToCancelAction() { return HoldToCancelAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetNavigateUpAction() { return NavigateUpAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetNavigateDownAction() { return NavigateDownAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetNavigateRightAction() { return NavigateRightAction; }

	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Actions")
	UInputAction* GetNavigateLeftAction() { return NavigateLeftAction; }



protected:
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* AcceptPressedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* AcceptReleasedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* CancelPressedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* CancelReleasedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* HoldToConfirmAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* HoldToCancelAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* NavigateUpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* NavigateDownAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* NavigateRightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Actions")
	UInputAction* NavigateLeftAction;

	/*
	* Enhanced Player Input Mapping Contexts
	*/
public:
	UFUNCTION(BlueprintCallable, Category = "Enhanced Player Input | Mapping Contexts")
	UInputMappingContext* GetHUDMappingContext() { return HUDMappingContext; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Player Input | Mapping Contexts")
	UInputMappingContext* HUDMappingContext;
};