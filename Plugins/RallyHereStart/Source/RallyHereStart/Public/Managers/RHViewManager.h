#pragma once
#include "CoreMinimal.h"
#include "Components/CanvasPanel.h"
#include "Engine/DataTable.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHViewManager.generated.h"

//$$ KAB - Heavily modified to use GameplayTags instead of FNames for Route Management, we are ignoring updates to this file

// These assume that the further the layer, the higher the layer is in the view
UENUM(BlueprintType)
enum class EViewManagerLayer: uint8
{
    Base,
    Modal
};

UENUM(BlueprintType)
enum class EViewManagerTransitionState : uint8
{
    Idle,
    Loading,
    AnimatingHide,
    AnimatingShow,
    Locked
};

UENUM(BlueprintType)
enum class EViewRouteRedirectionPhase : uint8
{
    VIEW_ROUTE_REDIRECT_None                UMETA(DisplayName="None"),
    VIEW_ROUTE_REDIRECT_ApplicationLaunch   UMETA(DisplayName="Application Launch"),
    VIEW_ROUTE_REDIRECT_AccountLogin        UMETA(DisplayName="Account Login"),
    VIEW_ROUTE_REDIRECT_AlwaysCheck         UMETA(DisplayName="Always Check")
};

//$$ JJJT: Begin Addition - Navigation type
UENUM(BlueprintType)
enum class EViewRouteNavigationType : uint8
{
	CommonUI,
	RallyHere
};
//$$ JJJT: End Addition

USTRUCT()
struct FViewRouteRedirectData
{
    GENERATED_BODY()

    // Route Name that this redirector will send the user to
    UPROPERTY()
    FGameplayTag Route;

    // Order this route is checked to load
    UPROPERTY()
    int32 CheckOrder;

    // If this is set, the route will open as well as the base route
    UPROPERTY()
    bool OpenOverOriginal;

    // This is the View Redirector this redirector uses
    UPROPERTY()
    URHViewRedirecter* Redirector;

	FViewRouteRedirectData() :
		CheckOrder(0),
		OpenOverOriginal(false),
		Redirector(nullptr)
	{ }
};

USTRUCT(BlueprintType)
struct FStickyWidgetData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category="Stick Widget Data")
    FName StickyWidgetName;

    UPROPERTY(BlueprintReadWrite, Category="Stick Widget Data")
    URHWidget* Widget;

	FStickyWidgetData() :
		Widget(nullptr)
	{ }
};

USTRUCT(BlueprintType)
struct FViewRoute : public FTableRowBase
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "View Route", meta = (Categories = "View"))
	FGameplayTag RouteTag;
	UPROPERTY(EditAnywhere, Category="View Route")
    TSubclassOf<URHWidget> ViewWidget;
    UPROPERTY(EditAnywhere, Category="View Route")
    TArray<FName> ViewStickyWidgets;
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="View Route")
    EViewManagerLayer ViewLayer;
    UPROPERTY(EditAnywhere, Category="View Route")
    bool IsDefaultRoute;
    UPROPERTY(EditAnywhere, Category="View Route")
    bool ShouldPreload;
	// Flags if a route should be accessible while not logged in or not (NOTE: this flag does nothing on its own, requires outside logic to make a re-route if required)
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="View Route")
	bool RequiresLoggedIn;
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="View Route")
    bool AlwaysShowContextBar;
    // If this is set, then attempt to load this route during this time instead of other routes
    UPROPERTY(EditAnywhere, Category = "Redirection")
    EViewRouteRedirectionPhase RedirectionPhase;
    // If is a non-negative number, this route will be loaded in the order specified as a redirection
    UPROPERTY(EditAnywhere, Category = "Redirection")
    int32 RedirectionPhaseOrder;
    // This is an optional class that can define extra checks to redirect load a scene
    UPROPERTY(EditAnywhere, Category = "Redirection")
    TSubclassOf<URHViewRedirecter> ViewRedirector;
    // If this is set, the route will open as well as the base route
    UPROPERTY(EditAnywhere, Category = "Redirection")
    bool OpenOverOriginal;
	// While this screen is the top scene on its layer, orders will be blocked
    UPROPERTY(EditAnywhere, Category="View Route")
    bool BlockOrders;

	//$$ JJJT: Begin Addition - input mode selection
	/// Does this screen use CommonUI navigation, or only allow RallyHere?
    UPROPERTY(EditAnywhere, Category="View Route")
    EViewRouteNavigationType NavigationType = EViewRouteNavigationType::RallyHere;
	//$$ JJJT:End Addition


    FViewRoute()
    : ViewLayer(EViewManagerLayer::Base)
    , IsDefaultRoute(false)
    , ShouldPreload(false)
	, RequiresLoggedIn(true)
    , AlwaysShowContextBar(false)
    , RedirectionPhase(EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_None)
    , RedirectionPhaseOrder(INDEX_NONE)
    , OpenOverOriginal(false)
	, BlockOrders(false)
    {
    }
    ~FViewRoute() {}
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHGenericRouteDataObject : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category="Route Data")
    FString StringValue;

    UPROPERTY(BlueprintReadWrite, Category="Route Data")
    int32 IntValue;

    UPROPERTY(BlueprintReadWrite, Category="Route Data")
    FGameplayTag NameValue;
};

UCLASS()
class RALLYHERESTART_API URHViewRedirecter : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    virtual bool ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData) { return true; };
};

UCLASS()
class URHViewLayer : public UObject
{
    GENERATED_BODY()

public:
    // The views will be put on this canvas.
    UPROPERTY()
    UCanvasPanel* DisplayTarget;

    // Reference back to out ViewManager
    UPROPERTY()
    URHViewManager* MyManager;

    // Initializes the view
    void Initialize(UCanvasPanel* ViewTarget, URHViewManager* ViewManager, EViewManagerLayer Type);

    // Replaces the current view route with a new one
    bool ReplaceRoute(const FGameplayTag& RouteTag, const TArray<FGameplayTag>& AdditionalRouteTags, bool ForceTransition = false, UObject* Data = nullptr);
    bool PushRoute(const FGameplayTag& RouteTag, const TArray<FGameplayTag>& AdditionalRouteTags, bool ForceTransition = false, UObject* Data = nullptr);
    bool PopRoute(bool ForceTransition = false); //this should not remove the last element in the current stack
    // Remove the given route from the stack
    bool RemoveRoute(const FGameplayTag& RouteTag, bool ForceTransition = false);
    bool SwapRoute(const FGameplayTag& RouteTag, const FGameplayTag& SwapTargetRouteTag = FGameplayTag::EmptyTag, bool ForceTransition = false);
	void ClearRoutes();

	// Returns if the stack has this route at any level
	bool ContainsRoute(const FGameplayTag& RouteTag);

    // Adds a view widget to the given layer
    void StoreViewWidget(const FGameplayTag& RouteTag, URHWidget* Widget);

    // Returns and pending data for a given transition route
    bool GetPendingRouteData(const FGameplayTag& RouteTag, UObject*& Data);

    // Sets the pending route data for a route, used when transitioning back and not using a AddViewRoute call
    void SetPendingRouteData(const FGameplayTag& RouteTag, UObject* Data);

    FORCEINLINE EViewManagerTransitionState GetCurrentTransitionState() const { return CurrentTransitionState; }
    FORCEINLINE FGameplayTag GetCurrentRoute() const { return CurrentRouteStack.Num() > 0 ? CurrentRouteStack.Top() : FGameplayTag::EmptyTag; }
    FORCEINLINE URHWidget* GetCurrentRouteWidget() const { return CurrentRouteStack.Num() > 0 ? RouteWidgetMap[CurrentRouteStack.Top()] : nullptr; }
    FORCEINLINE TArray<FGameplayTag> GetCurrentRouteStack() const { return CurrentRouteStack; }
    FORCEINLINE FGameplayTag GetCurrentTransitionRoute() const { return CurrentTransitionRouteStack.Num() > 0 ? CurrentTransitionRouteStack.Top() : FGameplayTag::EmptyTag; }
    FORCEINLINE URHWidget* GetCurrentTransitionRouteWidget() const { return CurrentTransitionRouteStack.Num() > 0 ? RouteWidgetMap[CurrentTransitionRouteStack.Top()] : nullptr; }
    FORCEINLINE TArray<FGameplayTag> GetCurrentTransitionRouteStack() const { return CurrentTransitionRouteStack; }
    FORCEINLINE EViewManagerLayer GetLayerType() const { return LayerType; }
    const TMap<FGameplayTag, URHWidget*>& GetWidgetMap() { return RouteWidgetMap; }

    void SetDefaultRoute(const FGameplayTag& RouteTag) { DefaultRoute = RouteTag; }
	FORCEINLINE FGameplayTag GetDefaultRoute() const { return DefaultRoute; }
    void AddRouteToLayer(const FGameplayTag& RouteTag, const FViewRoute& Route) { Routes.Add(RouteTag, Route); }

protected:
    // The state we are in of the transition
    UPROPERTY()
    EViewManagerTransitionState CurrentTransitionState;

    // The current route stack
    UPROPERTY()
    TArray<FGameplayTag> CurrentRouteStack;

    // The new route stack we are transitioning to
    UPROPERTY()
    TArray<FGameplayTag> CurrentTransitionRouteStack;

    // Map of every Route to the widget it creates
    UPROPERTY(Transient)
    TMap<FGameplayTag, URHWidget*> RouteWidgetMap;

    // The default route for a given scene stack
    UPROPERTY()
    FGameplayTag DefaultRoute;

    // Type of view layer
    EViewManagerLayer LayerType;

    // Map of route data for pending routes
    UPROPERTY(Transient)
    TMap<FGameplayTag, UObject*> PendingRouteData;

    UPROPERTY(Transient)
	TMap<FGameplayTag, FViewRoute> Routes;

    // Clears out the pending transition
    void ClearPendingTransition();

    // Finalizes a transition and cleans up any lingering states
    void FinalizeTransition();

    // Checks if the layer is currently displaying the default route
    bool IsAtDefaultRoute() const { return GetCurrentRoute() == DefaultRoute; }

    // Starts an actual route change on the layer
    bool RequestRouteStackChange(const TArray<FGameplayTag>& RouteStack, bool ForceTransition);

    // Called when the old route has finished animating out
    UFUNCTION()
    void GoToRoute_HandleHideFinished(URHWidget* Widget);

    // Called when the new route has finished animating in
    UFUNCTION()
    void GoToRoute_HandleShowFinished(URHWidget* Widget);

    // Called when we start transitioning to a new route
    UFUNCTION()
    void GoToRoute_InternalShowStep();

    // Returns whether or not the route exists in the route table.
    UFUNCTION(BlueprintCallable, Category="View Layer")
    bool IsRouteValid(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FViewStateChanged, const FGameplayTag&, CurrentRoute, const FGameplayTag&, PreviousRoute, EViewManagerLayer, Layer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FViewStateChangeStarted, const FGameplayTag&, CurrentRoute, const FGameplayTag&, NextRoute, EViewManagerLayer, Layer);

/**
* 
*/
UCLASS(BlueprintType)
class RALLYHERESTART_API URHViewManager : public UObject
{
    GENERATED_BODY()

public:
    URHViewManager();

	/* These members are ExposedOnSpawn in BP, need this to set them manually in native. */
	void SetMembersOnConstruct(ARHHUDCommon* InHudRef, const TArray<UCanvasPanel*>& InCanvasPanels, const TArray<FStickyWidgetData>& InStickyWidgetData, UDataTable* InRouteTable, bool InCanBaseLayerBeEmpty);

    // Initialized all of the view layers, sticky widgets, etc that were passed in on construction
    UFUNCTION(BlueprintCallable, Category="View State Manager")
    void Initialize();

    void Uninitialize();

    void PostLogin();
    void PostLogoff();

    // Array of all view layers that views can be assigned to
    UPROPERTY()
    TArray<URHViewLayer*> ViewLayers;

    // Map of every route to the Sticky Widgets it shows
    UPROPERTY()
    TMap<FName, URHWidget*> StickyWidgetMap;

	UFUNCTION(BlueprintCallable, Category = "View State Manager")
	bool GetViewRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, FViewRoute& ViewRoute);

    UFUNCTION(BlueprintCallable, Category="View State Manager")
    virtual bool ReplaceRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, bool ForceTransition = false, UObject* Data = nullptr);  //$$ JJJT - Made virtual

    UFUNCTION(BlueprintCallable, Category="View State Manager")
    virtual bool PushRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, bool ForceTransition = false, UObject* Data = nullptr); //$$ JJJT - Made virtual

    UFUNCTION(BlueprintCallable, Category="View State Manager")
    virtual bool PopRoute(bool ForceTransition = false); //this should not remove the last element in the current stack	//$$ JJJT - Made virtual

    // Remove the given route from the stack
    UFUNCTION(BlueprintCallable, Category="View State Manager")
    virtual bool RemoveRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, bool ForceTransition = false); //$$ JJJT - Made virtual

    UFUNCTION(BlueprintCallable, Category="View State Manager")
    bool SwapRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, UPARAM(meta = (Categories = "View")) FGameplayTag SwapTargetRouteTag, bool ForceTransition = false);

	void ClearRoutes();

    UFUNCTION(BlueprintPure, Category="View State Manager")
	const FGameplayTag GetTopViewRoute() const;

    UFUNCTION(BlueprintPure, Category="View State Manager")
    URHWidget* GetTopViewRouteWidget() const;

    UFUNCTION(BlueprintPure, Category="View State Manager")
    int32 GetViewRouteCount() const;

    UFUNCTION(BlueprintCallable, Category="View State Manager")
    bool GetPendingRouteData(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, UObject*& Data);

	// Returns whether the given route name is identifiable and is currently anywhere in its view layer's stack
	UFUNCTION(BlueprintPure, Category="View State Manager")
	bool ContainsRoute(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag);

    // Sets the pending route data for a route, used when transitioning back and not using a AddViewRoute call
    UFUNCTION(BlueprintCallable, Category="View State Manager")
    void SetPendingRouteData(UPARAM(meta = (Categories = "View")) FGameplayTag RouteTag, UObject* Data);

    UFUNCTION(BlueprintPure, Category="View State Manager")
	const FGameplayTag GetCurrentRoute(EViewManagerLayer Layer) const;

    UFUNCTION(BlueprintPure, Category="View State Manager")
	const FGameplayTag GetCurrentTransitionRoute(EViewManagerLayer Layer) const;

    ARHHUDCommon* GetHudRef() const { return HudRef; };

    UPROPERTY(BlueprintAssignable, Category = "View State Manager")
    FViewStateChanged OnViewStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "View State Manager")
    FViewStateChangeStarted OnViewStateChangeStarted;

	UFUNCTION(BlueprintPure, Category = "View State Manager")
	EViewManagerLayer GetTopLayer() const;

	UFUNCTION(BlueprintPure, Category = "View State Manager")
	const FGameplayTag GetDefaultRouteForLayer(EViewManagerLayer LayerType) const;

    UFUNCTION(BlueprintPure, Category = "View State Manager")
    bool HasCompletedRedirectFlow(EViewRouteRedirectionPhase RedirectPhase) const;

    UFUNCTION(BlueprintPure, Category = "View State Manager")
    bool IsBlockingOrders() const;

	UFUNCTION(BlueprintPure, Category = "View State Manager")
	bool IsLayerIdle(EViewManagerLayer LayerType) const;

	UFUNCTION(BlueprintPure, Category = "View State Manager")
	bool IsEveryLayerIdle() const;

	bool CanBaseLayerBeEmpty() const { return bCanBaseLayerBeEmpty; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "View State Manager", meta = (ExposeOnSpawn = "true"))
    ARHHUDCommon* HudRef;

    // Set of Canvas Panels used by each view layer
    UPROPERTY(BlueprintReadOnly, Category = "View State Manager", meta = (ExposeOnSpawn = "true"))
    TArray<UCanvasPanel*> CanvasPanels;

    // Set of all Sticky Widgets associated with the View Manager
    UPROPERTY(BlueprintReadOnly, Category = "View State Manager", meta = (ExposeOnSpawn = "true"))
    TArray<FStickyWidgetData> StickyWidgets;

	// If set load routes off this table during initialization
	UPROPERTY(BlueprintReadOnly, Category = "View State Manager", meta = (ExposeOnSpawn = "true", RequiredAssetDataTags = "RowStructure=/Script/RallyHereStart.FViewRoute"))
	TSoftObjectPtr<UDataTable> InitializationDataTableSoftPtr;

	// Can the base layer's stack ever be empty? E.g. no for lobby, yes for gameplay
	UPROPERTY(BlueprintReadOnly, Category = "View State Manager", meta = (ExposeOnSpawn = "true"))
	bool bCanBaseLayerBeEmpty;

	// Map of all route information used to build out the layers
	UPROPERTY(BlueprintReadOnly, Category = "View State Manager")
	TMap<FGameplayTag, FViewRoute> Routes;

    // List of always check routes and their redirect checkers
    UPROPERTY(Transient)
    TArray<FViewRouteRedirectData> AlwaysCheckRouteData;

    // Sets up the routes in the route Data Table
    UFUNCTION()
    void InitializeRoutes();

    // Map a name to a Widget reference so that ViewRoutes can reference sticky widgets by these names
    void RegisterStickyWidget(FStickyWidgetData* WidgetData);

    // Checks if there is a different route to direct the player to, and if so swap it in
	const FGameplayTag ReplaceTargetRoute(const FGameplayTag& RouteTag, UObject*& Data, bool& bOpenOriginal);

    // Gets the layer object for where the given route lives
    URHViewLayer* GetLayerForRoute(const FGameplayTag& RouteTag);
    URHViewLayer* GetLayerForRoute(FViewRoute* Route);

	// Gets the layer object by enum type
	URHViewLayer* GetLayerByType(EViewManagerLayer LayerType) const;

	bool ShouldUseAccountLoginPhase() const;
};