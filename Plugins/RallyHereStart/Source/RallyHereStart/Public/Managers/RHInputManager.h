#pragma once
#include "Engine/DataTable.h"
#include "Shared/Widgets/RHWidget.h"
#include "GameFramework/RHPlayerInput.h"
#include "Framework/Application/NavigationConfig.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "RHInputManager.generated.h"

//$$ KAB - Heavily modified to use GameplayTags instead of FNames for Route Management, we are ignoring updates to this file

RALLYHERESTART_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_View_Global);

UENUM()
enum class ERHLastInputType : uint8
{
	ERHLastInputType_Up,
	ERHLastInputType_Down,
	ERHLastInputType_Left,
	ERHLastInputType_Right,
};

/** Base navigation for slate applications that use the RHInputManager. */
class FRHNavigationConfig : public FNavigationConfig
{
public:
	FRHNavigationConfig() : FNavigationConfig()
    {
        KeyEventRules.Empty();
		bAnalogNavigation = false;
		bKeyNavigation = false;
    }
};

class FRHInputFocusWidget : public TSharedFromThis<FRHInputFocusWidget>
{
public:
    TWeakObjectPtr<UWidget> Widget;
    TWeakObjectPtr<UWidget> Up;
    TWeakObjectPtr<UWidget> Down;
    TWeakObjectPtr<UWidget> Left;
    TWeakObjectPtr<UWidget> Right;
};

USTRUCT()
struct FRHInputFocusGroup
{
    GENERATED_USTRUCT_BODY()

    TArray< TSharedRef<FRHInputFocusWidget> > Widgets;

    int32 FocusGroupIndex;

    TWeakPtr<FRHInputFocusWidget> DefaultFocusWidget;
    TWeakPtr<FRHInputFocusWidget> CurrentFocusWidget;

	FRHInputFocusGroup() : FocusGroupIndex(-1) { }
};

USTRUCT()
struct FRHInputFocusDetails
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TArray<FRHInputFocusGroup> FocusGroups;

    UPROPERTY()
    int32 DefaultFocusGroupIndex;

    UPROPERTY()
    int32 CurrentFocusGroupIndex;

	FRHInputFocusDetails() : DefaultFocusGroupIndex(-1), CurrentFocusGroupIndex(-1) { }
};

// Determines how a given Context Action is interacted with
UENUM(BlueprintType)
enum class EContextActionType : uint8
{
	ContextActionTypeStandard,
	ContextActionTypeCycle,
	ContextActionTypeHoldRelease,
};

// Used by the Context Bar to determine how to anchor the context action if it is a displayable prompt
UENUM(BlueprintType)
enum class EContextPromptAnchoring : uint8
{
	AnchorLeft,
	AnchorRight,
	AnchorCenter
};

USTRUCT(BlueprintType)
struct FContextAction : public FTableRowBase
{
	GENERATED_BODY()

	// The text to display on this prompt on a Context Bar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	FText Text;

	// This is the Action the Binding is tied to
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	UInputAction* Action;

	// This is the secondary Action bound to restore the state from the main Action.
	// Used for entries of type ContextActionTypeCycle.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Context Action")
	UInputAction* AltAction;

	// This is the list of input types to display this binding for
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	TArray<TEnumAsByte<RH_INPUT_STATE>> ValidInputTypes;

	// The Sort Priority of this action (lower is first)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	int32 SortOrder;

	// The positioning of the prompt on a Context Bar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	EContextPromptAnchoring Anchor;

	// This is the way the context behaves
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	EContextActionType ActionType;

	// This is a widget/class to use to display the prompt on a Context Bar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	TSubclassOf<URHWidget> PromptWidget;

	// If the context is a Hold Action, this is the duration to fire the success event
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	float HoldDuration;

	// This context exists for capturing input and routing it but does not display on the bar,
	// In general is used for bumpers or other on screen actions via gamepad
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Context Action")
	bool IsHidden;

	bool operator<(const FContextAction& Other) const
	{
		return (SortOrder < Other.SortOrder);
	}

	bool operator>(const FContextAction& Other) const
	{
		return (SortOrder > Other.SortOrder);
	}

	FContextAction() :
		Action(nullptr),
		AltAction(nullptr),
		SortOrder(0),
		Anchor(EContextPromptAnchoring::AnchorLeft),
		ActionType(EContextActionType::ContextActionTypeStandard),
		HoldDuration(0.0f),
		IsHidden(false)
	{ }

	~FContextAction() {}
};

UCLASS(BlueprintType)
class UContextActionData : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	bool operator<(const UContextActionData& Other) const
	{
		return (RowData < Other.RowData);
	}

	bool operator>(const UContextActionData& Other) const
	{
		return (RowData > Other.RowData);
	}

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const { return bIsTicking; }

	// Calls the ContextAction bound to this action
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void TriggerContextAction();

	// Calls the CycleContextAction bound to this action
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void TriggerCycleContextAction(bool bNext);

	UFUNCTION(BlueprintCallable, Category="Context Action")
	void TriggerHoldReleaseContextAction(EContextActionHoldReleaseState Status);

	UFUNCTION()
	void TriggerCycleContextActionPrev() { TriggerCycleContextAction(false); }
	UFUNCTION()
	void TriggerCycleContextActionNext() { TriggerCycleContextAction(true); }

	UFUNCTION()
	void StartTriggerHoldAction();
	UFUNCTION()
	void ClearTriggerHoldAction();

	// Name of the Row while it is in the Data Table
	UPROPERTY(BlueprintReadOnly, Category="Context Action")
	FName RowName;

	// Extra data for context display
	UPROPERTY(BlueprintReadOnly, Category="Context Action")
	FText FormatAdditive;

	// Row Data for the Context Action
	UPROPERTY(BlueprintReadOnly, Category="Context Action")
	FContextAction RowData;

	// Callback Delegate that can be triggered on a ContextAction
	UPROPERTY()
	FOnContextAction OnContextAction;

	// Callback Delegate that can be triggered on a CycleContextAction
	UPROPERTY()
	FOnContextCycleAction OnCycleAction;

	// Callback Delegate that can be triggered on a ContextActionTypeHoldRelease that tells the percentage into the hold on a tick basis
	UPROPERTY()
	FOnContextHoldActionUpdate OnHoldActionUpdate;

	// Callback Delegate that can be triggered on a ContextActionTypeHoldRelease that tells if the hold succeeds or is canceled
	UPROPERTY()
	FOnContextHoldAction OnHoldReleaseAction;

protected:
	// Tracking Ticking
	bool bIsTicking;
	float fTickDuration;
};

USTRUCT(BlueprintType)
struct FRouteContextInfo
{
	GENERATED_BODY()

public:
	// Clears a specific context off the bar if it is on it
	bool ClearContextAction(FName ContextName);

	// Clears all of the contexts off the context bar
	bool ClearAllContextActions();

	// Adds a given context on the context bar
	bool AddContextAction(FName ContextName, UDataTable* ContextActionTable, FText FormatAdditive = FText());

	// Sets an event callback for a given context being activated
	void SetContextAction(FName ContextName, const FOnContextAction& EventCallback);

	// Sets an event callback for a given cycle context being activated
	void SetContextCycleAction(FName ContextName, const FOnContextCycleAction& EventCallback);

	// Sets an event callback for a given cycle context being activated
	void SetContextHoldReleaseAction(FName ContextName, const FOnContextHoldActionUpdate& UpdateCallback, const FOnContextHoldAction& EventCallback);

	UPROPERTY()
	TArray<UContextActionData*> ActionData;

protected:
	// Locates the Context Action based on the name provided
	bool FindRowByName(UDataTable* ContextActionTable, FName ContextName, FContextAction& ContextActionRow);
};

UCLASS(config = game)
class RALLYHERESTART_API URHInputManager : public UObject
{
    GENERATED_UCLASS_BODY()

public:
    void Initialize(class ARHHUDCommon* Hud, bool bUseRHNavigation); //$$ JJJT: Modification - added parameter
    //$$ JJJT: Begin Addition - navigation config adjustments
    void Uninitialize(bool bUseRHNavigation);
    void SetNavigationConfig();
    void ClearNavigationConfig();
    //$$ JJJT: End Addition

    void BindParentWidget(URHWidget* ParentWidget, int32 DefaultFocusGroup);
    void UnbindParentWidget(URHWidget* ParentWidget);
    void RegisterWidget(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup, UWidget* Up, UWidget* Down, UWidget* Left, UWidget* Right);
    void UpdateWidgetRegistration(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup, UWidget* Up, UWidget* Down, UWidget* Left, UWidget* Right);
    void UnregisterWidget(URHWidget* ParentWidget, UWidget* Widget);
    void UnregisterFocusGroup(URHWidget* ParentWidget, int32 FocusGroup);
    void SetFocusToGroup(URHWidget* ParentWidget, int32 FocusGroup, bool KeepLastFocus);
    void SetFocusToWidgetOfGroup(URHWidget* ParentWidget, int32 FocusGroup, URHWidget* Widget);
    void SetDefaultFocusForGroup(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup);
    UWidget* SetFocusToThis(URHWidget* ParentWidget);
    UWidget* GetCurrentFocusForGroup(URHWidget* ParentWidget, int32 FocusGroup);
    bool GetCurrentFocusGroup(URHWidget* ParentWidget, int32& OutFocusGroup);
	void InheritFocusGroupFromWidget(URHWidget* TargetWidget, int32 TargetFocusGroupNum, URHWidget* SourceWidget, int32 SourceFocusGroupNum);

    UFUNCTION()
    void HandleModeChange(RH_INPUT_STATE mode);

	void NavigateUp();
	void NavigateDown();
	void NavigateLeft();
	void NavigateRight();

	void OnNavigateUpAction(const FInputActionValue& Value) { NavigateUp(); }
	void OnNavigateDownAction(const FInputActionValue& Value) { NavigateDown(); }
	void OnNavigateLeftAction(const FInputActionValue& Value) { NavigateLeft(); }
	void OnNavigateRightAction(const FInputActionValue& Value) { NavigateRight(); }

    void OnAcceptPressed(const FInputActionValue& Value);
    void OnAcceptReleased(const FInputActionValue& Value);
	void OnCancelPressed(const FInputActionValue& Value);
	void OnCancelReleased(const FInputActionValue& Value);

    void SetNavigationEnabled(bool enabled);

    UFUNCTION(BlueprintPure, Category="Focus")
    bool GetFocusedWidget(URHWidget* ParentWidget, UWidget*& FocusWidget);

protected:

	void BindBaseHUDActions();

    TSharedRef<FRHInputFocusWidget>* GetWidget(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup);
    int32 GetFocusedWidget(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget>& FocusWidget);
    UWidget* ChangeFocus(TSharedRef<FRHInputFocusWidget> NewFocus);
    virtual bool IsUIFocused();

    bool NavigateUp_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup);
    bool NavigateDown_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup);
    bool NavigateLeft_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup);
    bool NavigateRight_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup);

    UPROPERTY()
    TMap<URHWidget*, FRHInputFocusDetails> InputFocusData;

    TWeakObjectPtr<class ARHHUDCommon> MyHud;
    TWeakPtr<FRHInputFocusWidget> LastFocusedWidget;
    TWeakObjectPtr<URHWidget> LastFocusedParentWidget;

    RH_INPUT_STATE LastInputState;

    UPROPERTY(Transient, DuplicateTransient)
    UEnhancedInputComponent* InputComponent;

    //prevents the navigation failure events from causing infinite recursion.  We never need to go more than one layer deep.
    bool NavFailRecursionGate;

    TSharedPtr<FRHInputFocusWidget> InputLockWidget;

/*
* Key Mapping
*/
public:

	UFUNCTION(BlueprintCallable, Category = "Key Binding")
	bool GetButtonForInputAction(UInputAction* Action, FKey& Button, bool IsGamepadKey = false);

	UFUNCTION(BlueprintCallable, Category = "Key Binding")
	bool GetAllButtonsForInputAction(UInputAction* Action, TArray<FKey>& Buttons);

protected:

	TArray<FKey> GetBoundKeysForInputActionInMappingContext(UInputAction* Action, UInputMappingContext* MappingContext);

/*
* Navigation input throttling
*/
public:
    UWorld* GetWorld() const override;

    UFUNCTION()
    void ClearNavInputThrottled();

    UFUNCTION()
    void ClearNavInputDebouncedThrottled();

protected:
    void SetNavInputThrottled(ERHLastInputType InputType);
    bool IsNavInputThrottled(ERHLastInputType InputType);
    bool IsNavInputDebounceThrottled(ERHLastInputType InputType);

private:
    FTimerHandle NavInputThrottleTimerHandle;
    FTimerHandle NavInputDebounceThrottleTimerHandle;

	ERHLastInputType LastInputType;

/*
* Context Actions
*/
public:
	// Event that is broadcast whenever the context actions on a given route are updated or the route changes
	FOnContextActionsUpdated OnContextActionsUpdated;

	// Route used for storing global context action data
	// TODO: Evaluate if we want to hide this entirely behind function calls.
	UPROPERTY(BlueprintReadOnly, Category="Context Action")
	FGameplayTag GlobalRouteTag;

	// Clears a specific context for a given route
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void ClearContextAction(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, FName ContextName);

	// Clears all of the contexts for a given route
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void ClearAllContextActions(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag);

	// Adds a given context for a given route
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void AddContextAction(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, FName ContextName, FText FormatAdditive = FText());

	// Adds multiple contexts for a given route
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void AddContextActions(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, TArray<FName> ContextNames);

	// Sets the new active route and updates active contexts
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void SetActiveRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag);

	// Gets the new active route
	UFUNCTION(BlueprintPure, Category="Context Action")
	const FGameplayTag& GetCurrentRoute() const { return OverrideRouteStack.Num() > 0 ? OverrideRouteStack.Last() : ActiveRouteTag; }

	// Sets an override route that displays on top of the active route, used for non-route popups (Popup Manager, etc.)
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void PushOverrideRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag);

	UFUNCTION(BlueprintCallable, Category="Context Action")
	const FGameplayTag PopOverrideRoute();

	// Removed a specified Override Route if it exists
	UFUNCTION(BlueprintCallable, Category="Context Action")
	bool RemoveOverrideRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag);

	// Clears the override route, returning the active route contexts to being active
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void ClearOverrideRouteStack();

	// Sets an event callback for a given context being activated
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void SetContextAction(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, FName ContextName, const FOnContextAction& EventCallback);

	// Sets an event callback for a given cycle context being activated
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void SetContextCycleAction(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, FName ContextName, const FOnContextCycleAction& EventCallback);

	// Sets an event callback for a given hold and release context being activated
	UFUNCTION(BlueprintCallable, Category="Context Action")
	void SetContextHoldReleaseAction(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, FName ContextName, const FOnContextHoldActionUpdate& UpdateCallback, const FOnContextHoldAction& EventCallback);

	// Gets the Active context actions
	bool GetActiveContextActions(TArray<UContextActionData*>& TopRouteActions, TArray<UContextActionData*>& GlobalActions, FGameplayTag& CurrentRouteTag);

protected:

	// MappingContext that handles all context actions so they can be evaluated before common input
	UPROPERTY(Transient, DuplicateTransient)
	UInputMappingContext* ContextualMappingContext;

	/** Path to the DataTable to load, configurable per game. If empty, it will not spawn one */
	UPROPERTY(Config)
	FSoftObjectPath ContextActionDataTableClassName;

	// Table of all available Context Actions
	UPROPERTY(Transient)
	UDataTable* ContextActionDT;

	// Stores the given context actions by route
	UPROPERTY()
	TMap<FGameplayTag, FRouteContextInfo> RouteContextInfoMap;

	// The routes whose context actions are currently valid
	UPROPERTY()
	FGameplayTag ActiveRouteTag;

	// If routes are present here, the topmost route's context options show on the context bar instead of ActiveRoute's context options.
	// We currently support setting ActiveRoute "underneath" an override route, so ActiveRoute remains separate.
	UPROPERTY(BlueprintReadOnly, Category="Context Action")
	TArray<FGameplayTag> OverrideRouteStack;

	// Returns the currently displayed route (Override or Active)
	UFUNCTION()
	const FGameplayTag& GetCurrentContextRoute() const;

	// Sets up Input Bindings for the given Action Data
	UFUNCTION()
	void SetInputActions(TArray<UContextActionData*> ActionData);

	// Updates the active contexts and notifies other components they have been updated
	void UpdateActiveContexts();

	// Adds a mapping to ContextualMappingContext, mimicking the key-bind on the highest priority existing active MappingContext
	void MapContextualAction(UInputAction* Action);
};