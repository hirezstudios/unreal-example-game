// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RH_Common.h"
#include "Managers/RHViewManager.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "DataFactories/RHLoginDataFactory.h"

//$$ KAB - Heavily modified to use GameplayTags instead of FNames for Route Management, we are ignoring updates to this file

// Tracks the next to be viewed route in the each route redirection phase
static TMap<EViewRouteRedirectionPhase, int32> ViewRedirectionDisplayCounter;

URHViewManager::URHViewManager() : Super()
{
    if (ViewRedirectionDisplayCounter.Num() == 0)
    {
        // Default the view routes displays to turn off None, and turn on the rest at the first index
        ViewRedirectionDisplayCounter.Add(EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_None, INDEX_NONE);
        ViewRedirectionDisplayCounter.Add(EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AccountLogin, 0);
        ViewRedirectionDisplayCounter.Add(EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_ApplicationLaunch, 0);
        ViewRedirectionDisplayCounter.Add(EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AlwaysCheck, 0);
    }
}

void URHViewManager::SetMembersOnConstruct(ARHHUDCommon* InHudRef, const TArray<UCanvasPanel*>& InCanvasPanels, const TArray<FStickyWidgetData>& InStickyWidgetData, UDataTable* InRouteTable, bool InCanBaseLayerBeEmpty)
{
	HudRef = InHudRef;
	CanvasPanels = InCanvasPanels;
	StickyWidgets = InStickyWidgetData;
	bCanBaseLayerBeEmpty = InCanBaseLayerBeEmpty;

	// Pull Data Table into internal map
	if (InRouteTable != nullptr)
	{
		for (const FName& rowName : InRouteTable->GetRowNames())
		{
			if (FViewRoute* route = InRouteTable->FindRow<FViewRoute>(rowName, "Set Routes lookup."))
			{
				Routes.Add(route->RouteTag, *route);
			}
		}
	}
}

void URHViewManager::Initialize()
{
    for (int32 i = 0; i < StickyWidgets.Num(); ++i)
    {
        RegisterStickyWidget(&StickyWidgets[i]);
    }

    // TODO: For now we assume that a Panel is provided per layer and we don't skip any
    for (int32 i = 0; i < CanvasPanels.Num(); ++i)
    {
        if (URHViewLayer* NewLayer = NewObject<URHViewLayer>())
        {
            NewLayer->Initialize(CanvasPanels[i], this, EViewManagerLayer(i));
            ViewLayers.Add(NewLayer);
        }
    }

	if (!InitializationDataTableSoftPtr.IsNull())
	{
		// We load this synchronously because there may be code after this that needs it immediately and expects it setup
		const UDataTable* InitializationDataTable = InitializationDataTableSoftPtr.IsValid() ? InitializationDataTableSoftPtr.Get() : InitializationDataTableSoftPtr.LoadSynchronous();

		if (InitializationDataTable != nullptr)
		{
			for (const FName& rowName : InitializationDataTable->GetRowNames())
			{
				if (FViewRoute* route = InitializationDataTable->FindRow<FViewRoute>(rowName, "Set Routes lookup."))
				{
					Routes.Add(route->RouteTag, *route);
				}
			}
		}
	}

	InitializeRoutes();
}

void URHViewManager::Uninitialize()
{
    // Uninitialize all views still open
    for (int32 i = 0; i < ViewLayers.Num(); ++i)
    {
        if (URHViewLayer* ViewLayer = ViewLayers[i])
        {
            for (auto ViewMapIter(ViewLayer->GetWidgetMap().CreateConstIterator()); ViewMapIter; ++ViewMapIter)
            {
                URHWidget* Widget = ViewMapIter.Value();
                if (Widget)
                {
                    Widget->UninitializeWidget();
                }
            }
        }
    }
}

void URHViewManager::PostLogin()
{
    ViewRedirectionDisplayCounter[EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AccountLogin] = 0;
}

void URHViewManager::PostLogoff()
{
    ViewRedirectionDisplayCounter[EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AccountLogin] = 0;
}

void URHViewManager::InitializeRoutes()
{
	for (const TPair<FGameplayTag, FViewRoute>& pair : Routes)
	{
		// to do Preload rows and add those widgets already
		if (URHViewLayer* viewLayer = GetLayerForRoute(pair.Key))
		{
			viewLayer->AddRouteToLayer(pair.Key, pair.Value);

			if (pair.Value.ShouldPreload)
			{
				if (URHWidget* TargetRouteWidget = CreateWidget<URHWidget>(GetWorld(), pair.Value.ViewWidget))
				{
					TargetRouteWidget->SetRouteTag(pair.Key);
					viewLayer->StoreViewWidget(pair.Key, TargetRouteWidget);
				}
			}

			if (pair.Value.IsDefaultRoute)
			{
				viewLayer->SetDefaultRoute(pair.Value.RouteTag);
			}
		}

		if (pair.Value.RedirectionPhase == EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AlwaysCheck)
		{
			FViewRouteRedirectData RedirectData;

			RedirectData.CheckOrder = pair.Value.RedirectionPhaseOrder;
			RedirectData.OpenOverOriginal = pair.Value.OpenOverOriginal;
			RedirectData.Route = pair.Value.RouteTag;
			RedirectData.Redirector = pair.Value.ViewRedirector ? NewObject<URHViewRedirecter>(this, pair.Value.ViewRedirector) : nullptr;

			AlwaysCheckRouteData.Add(RedirectData);
		}
	}

    AlwaysCheckRouteData.Sort([](const FViewRouteRedirectData& A, const FViewRouteRedirectData& B) { return A.CheckOrder < B.CheckOrder; });
}

URHViewLayer* URHViewManager::GetLayerForRoute(const FGameplayTag& RouteTag)
{
	return GetLayerForRoute(Routes.Find(RouteTag));
}

URHViewLayer* URHViewManager::GetLayerForRoute(FViewRoute* Route)
{
    if (Route && (int32)Route->ViewLayer < ViewLayers.Num())
    {
        return ViewLayers[(int32)Route->ViewLayer];
    }

    return nullptr;
}

URHViewLayer* URHViewManager::GetLayerByType(EViewManagerLayer LayerType) const
{
	if ((int32)LayerType < ViewLayers.Num())
	{
		return ViewLayers[(int32)LayerType];
	}

	return nullptr;
}

bool URHViewManager::GetViewRoute(FGameplayTag Route, FViewRoute& ViewRoute)
{
	if (const FViewRoute* findRoute = Routes.Find(Route))
	{
		ViewRoute = *findRoute;
		return true;
	}

	return false;
}

bool URHViewManager::ReplaceRoute(FGameplayTag RouteTag, bool ForceTransition/* = false*/, UObject* Data /*= nullptr*/)
{
	UObject* InData = Data;
	bool OpenOriginalScene = false;
	const FGameplayTag ReplacementRouteTag = ReplaceTargetRoute(RouteTag, Data, OpenOriginalScene);
	TArray<FGameplayTag> AdditionalRoutes;

    if (ReplacementRouteTag.IsValid())
    {
		URHViewLayer* ViewLayerTarget = GetLayerForRoute(ReplacementRouteTag);

		if (OpenOriginalScene)
		{
			URHViewLayer* ViewLayerOriginal = GetLayerForRoute(RouteTag);
			
			// If we are opening something on the same layer, slide it under the current open, otherwise open both
			if (ViewLayerOriginal == ViewLayerTarget)
			{
				AdditionalRoutes.Add(RouteTag);
			}
			else
			{
				ViewLayerOriginal->ReplaceRoute(RouteTag, AdditionalRoutes, ForceTransition, InData);
			}
		}

		return ViewLayerTarget->ReplaceRoute(ReplacementRouteTag, AdditionalRoutes, ForceTransition, Data);
    }

    if (URHViewLayer* ViewLayerTarget = GetLayerForRoute(RouteTag))
    {
        return ViewLayerTarget->ReplaceRoute(RouteTag, AdditionalRoutes, ForceTransition, InData);
    }

    return false;
}

bool URHViewManager::PushRoute(FGameplayTag RouteTag, bool ForceTransition/* = false*/, UObject* Data /*= nullptr*/)
{
	// If we're pushing a non-Base route, but the Base layer is currently changing, ignore this route change
	// This rule currently only applies to pushes
	if (URHViewLayer* BaseLayer = GetLayerByType(EViewManagerLayer::Base))
	{
		if (URHViewLayer* ModifiedLayer = GetLayerForRoute(RouteTag))
		{
			if (ModifiedLayer->GetLayerType() != EViewManagerLayer::Base && BaseLayer->GetCurrentTransitionState() != EViewManagerTransitionState::Idle)
			{
				return false;
			}
		}
	}

	UObject* InData = Data;
	bool OpenOriginalScene = false;
	const FGameplayTag ReplacementRouteTag = ReplaceTargetRoute(RouteTag, Data, OpenOriginalScene);
	TArray<FGameplayTag> AdditionalRoutes;

	if (ReplacementRouteTag.IsValid())
	{
		URHViewLayer* ViewLayerTarget = GetLayerForRoute(ReplacementRouteTag);

		if (OpenOriginalScene)
		{
			URHViewLayer* ViewLayerOriginal = GetLayerForRoute(RouteTag);

			// If we are opening something on the same layer, slide it under the current open, otherwise open both
			if (ViewLayerOriginal == ViewLayerTarget)
			{
				AdditionalRoutes.Add(RouteTag);
			}
			else
			{
				ViewLayerOriginal->PushRoute(RouteTag, AdditionalRoutes, ForceTransition, InData);
			}
		}

		return ViewLayerTarget->PushRoute(ReplacementRouteTag, AdditionalRoutes, ForceTransition, Data);
	}

	if (URHViewLayer* ViewLayerTarget = GetLayerForRoute(RouteTag))
	{
		return ViewLayerTarget->PushRoute(RouteTag, AdditionalRoutes, ForceTransition, InData);
	}

    return false;
}

void URHViewManager::ClearRoutes()
{
	for (URHViewLayer* ViewLayer : ViewLayers)
	{
		ViewLayer->ClearRoutes();
	}
}

bool URHViewManager::ShouldUseAccountLoginPhase() const
{
	auto* MyHud = GetHudRef();
	
	if (MyHud != nullptr)
	{
		if (URHLoginDataFactory* LoginDataFactory = MyHud->GetLoginDataFactory())
		{
			return LoginDataFactory->GetCurrentLoginState() == ERHLoginState::ELS_LoggedIn;
		}
	}
	
	if (MyHud != nullptr)
	{
		auto* LPSS = MyHud->GetLocalPlayerSubsystem();
		if (LPSS != nullptr)
		{
			return LPSS->IsLoggedIn();
		}
	}

	return false;
}

const FGameplayTag URHViewManager::ReplaceTargetRoute(const FGameplayTag& RouteTag, UObject*& Data, bool& bOpenOriginal)
{
    ViewRedirectionDisplayCounter[EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AlwaysCheck] = 0;
	bOpenOriginal = false;

    EViewRouteRedirectionPhase CurrentRedirectPhase = EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_None;

    if (ShouldUseAccountLoginPhase())
    {
        CurrentRedirectPhase = EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_AccountLogin;
    }
    else
    {
        CurrentRedirectPhase = EViewRouteRedirectionPhase::VIEW_ROUTE_REDIRECT_ApplicationLaunch;
    }

    // Check if we should search routes for this phase
    if (ViewRedirectionDisplayCounter[CurrentRedirectPhase] > INDEX_NONE)
    {
        TMap<int32, TPair<FGameplayTag, FViewRoute>> PossibleRoutes;

		FGameplayTag RedirectRouteTag = FGameplayTag::EmptyTag;
        UObject* PopupSceneData = nullptr;

		for (const TPair<FGameplayTag, FViewRoute>& pair : Routes)
		{
			const FViewRoute& viewRoute = pair.Value;

            // If we are the next route to try and redirect to, check if we should be loaded
            if (viewRoute.RedirectionPhase == CurrentRedirectPhase)
            {
                if (viewRoute.RedirectionPhaseOrder == ViewRedirectionDisplayCounter[CurrentRedirectPhase])
                {
                    ViewRedirectionDisplayCounter[CurrentRedirectPhase]++;
                    if (!viewRoute.ViewRedirector)
                    {
                        RedirectRouteTag = pair.Key;
                    }
                    else
                    {
                        URHViewRedirecter* ViewRedirector = NewObject<URHViewRedirecter>(this, viewRoute.ViewRedirector);
                        if (ViewRedirector && ViewRedirector->ShouldRedirect(GetHudRef(), RouteTag, Data))
                        {
                            RedirectRouteTag = pair.Key;
                        }
                    }
                }
                else if (viewRoute.RedirectionPhaseOrder > ViewRedirectionDisplayCounter[CurrentRedirectPhase])
                {
                    // Build out the route list of possible things we things we need to redirect to, in case we fail to find the current number
                    PossibleRoutes.Add(viewRoute.RedirectionPhaseOrder, pair);
                }

                if (RedirectRouteTag.IsValid())
                {
					bOpenOriginal = viewRoute.OpenOverOriginal;
                    return RedirectRouteTag;
                }
            }
        }

        // If we still have routes to redirect to, check them all in order
        if (PossibleRoutes.Num())
        {
            PossibleRoutes.KeySort([](int32 A, int32 B) { return A < B; });
            TArray<TPair<FGameplayTag, FViewRoute>> PossibleRouteRows;
            PossibleRoutes.GenerateValueArray(PossibleRouteRows);

            for (const auto& pair : PossibleRouteRows)
            {
				const FViewRoute& viewRoute = pair.Value;
                if (!viewRoute.ViewRedirector)
                {
                    RedirectRouteTag = pair.Key;
                    ViewRedirectionDisplayCounter[CurrentRedirectPhase] = viewRoute.RedirectionPhaseOrder + 1;
                }
                else
                {
                    URHViewRedirecter* ViewRedirector = NewObject<URHViewRedirecter>(this, viewRoute.ViewRedirector);
                    if (ViewRedirector && ViewRedirector->ShouldRedirect(GetHudRef(), RouteTag, Data))
                    {
						RedirectRouteTag = pair.Key;
                        ViewRedirectionDisplayCounter[CurrentRedirectPhase] = viewRoute.RedirectionPhaseOrder + 1;
                    }
                }

				if (RedirectRouteTag.IsValid())
				{
					bOpenOriginal = viewRoute.OpenOverOriginal;
					return RedirectRouteTag;
				}
            }
        }

        // Turn off this view redirector if we make it to the very end of the list
        ViewRedirectionDisplayCounter[CurrentRedirectPhase] = INDEX_NONE;
    }

    if (AlwaysCheckRouteData.Num())
    {
        for (int32 i = 0; i < AlwaysCheckRouteData.Num(); ++i)
        {
			if (AlwaysCheckRouteData[i].Route != RouteTag)
			{
				if (AlwaysCheckRouteData[i].Redirector && AlwaysCheckRouteData[i].Redirector->ShouldRedirect(GetHudRef(), RouteTag, Data))
				{
					bOpenOriginal = AlwaysCheckRouteData[i].OpenOverOriginal;
					return AlwaysCheckRouteData[i].Route;
				}
			}
        }
    }

    return FGameplayTag::EmptyTag;
}

bool URHViewManager::RemoveRoute(FGameplayTag RouteTag, bool ForceTransition/* = false*/)
{
    if (URHViewLayer* ViewLayer = GetLayerForRoute(RouteTag))
    {
        return ViewLayer->RemoveRoute(RouteTag, ForceTransition);
    }

    return false;
}

bool URHViewManager::PopRoute(bool ForceTransition/* = false*/)
{
    // Pop a route from the top most layer (last in stack order)
    // TODO: Require a specific layer to be requested from
    for (int32 i = ViewLayers.Num() - 1; i >= 0; i--)
    {
        // Return true the first time we find a popable route
        if (ViewLayers[i] && ViewLayers[i]->PopRoute(ForceTransition))
        {
            return true;
        }
    }

    return false;
}

bool URHViewManager::SwapRoute(FGameplayTag RouteTag, FGameplayTag SwapTargetRouteTag, bool ForceTransition/* = false*/)
{
	UObject* Data = nullptr;
	bool OpenOriginalScene = false;
	const FGameplayTag ReplacementRouteTag = ReplaceTargetRoute(RouteTag, Data, OpenOriginalScene);
	TArray<FGameplayTag> AdditionalRouteTags;

	if (ReplacementRouteTag.IsValid())
	{
		URHViewLayer* ViewLayerTarget = GetLayerForRoute(ReplacementRouteTag);

		if (OpenOriginalScene)
		{
			URHViewLayer* ViewLayerOriginal = GetLayerForRoute(RouteTag);

			// If we are opening something on the same layer, slide it under the current open, otherwise open both
			if (ViewLayerOriginal == ViewLayerTarget)
			{
				AdditionalRouteTags.Add(RouteTag);
			}
			else
			{
				return ViewLayerTarget->SwapRoute(RouteTag, SwapTargetRouteTag, ForceTransition);
			}
		}

		return ViewLayerTarget->PushRoute(ReplacementRouteTag, AdditionalRouteTags, ForceTransition, Data);
	}

    if (URHViewLayer* ViewLayer = GetLayerForRoute(RouteTag))
    {
        return ViewLayer->SwapRoute(RouteTag, SwapTargetRouteTag, ForceTransition);
    }

    return false;
}

void URHViewManager::RegisterStickyWidget(FStickyWidgetData* WidgetData)
{
    if (WidgetData)
    {
        StickyWidgetMap.Add(WidgetData->StickyWidgetName, WidgetData->Widget);
    }
}

bool URHViewManager::GetPendingRouteData(FGameplayTag RouteTag, UObject*& Data)
{
    if (URHViewLayer* ViewLayer = GetLayerForRoute(RouteTag))
    {
        return ViewLayer->GetPendingRouteData(RouteTag, Data);
    }

    return false;
}

void URHViewManager::SetPendingRouteData(FGameplayTag RouteTag, UObject* Data)
{
    if (URHViewLayer* ViewLayer = GetLayerForRoute(RouteTag))
    {
        ViewLayer->SetPendingRouteData(RouteTag, Data);
    }
}

bool URHViewManager::ContainsRoute(FGameplayTag RouteTag)
{
	if (URHViewLayer* ViewLayer = GetLayerForRoute(RouteTag))
	{
		return ViewLayer->ContainsRoute(RouteTag);
	}

	return false;
}

const FGameplayTag URHViewManager::GetTopViewRoute() const
{
    FGameplayTag RouteTag = FGameplayTag::EmptyTag;

    for (int32 i = ViewLayers.Num() - 1; i >= 0; i--)
    {
        if (ViewLayers[i])
        {
            RouteTag = ViewLayers[i]->GetCurrentRoute();
        }

        if (RouteTag.IsValid())
        {
            break;
        }
    }

    return RouteTag;
}

URHWidget* URHViewManager::GetTopViewRouteWidget() const
{
    for (int32 i = ViewLayers.Num() - 1; i >= 0; i--)
    {
        if (URHViewLayer* const ViewLayer = ViewLayers[i])
        {
            if (ViewLayer->GetCurrentTransitionState() != EViewManagerTransitionState::Idle)
            {
                if (URHWidget* TopViewRouteWidget = ViewLayers[i]->GetCurrentTransitionRouteWidget())
                {
                    return TopViewRouteWidget;
                }
            }
            else
            {
                if (URHWidget* TopViewRouteWidget = ViewLayers[i]->GetCurrentRouteWidget())
                {
                    return TopViewRouteWidget;
                }
            }
        }
    }

    return nullptr;
}

int32 URHViewManager::GetViewRouteCount() const
{
    int32 Count = 0;

    for (int32 i = ViewLayers.Num() - 1; i >= 0; i--)
    {
        if (ViewLayers[i])
        {
            Count += ViewLayers[i]->GetCurrentRouteStack().Num();
        }
    }

    return Count;
}

const FGameplayTag URHViewManager::GetCurrentRoute(EViewManagerLayer Layer) const
{
    if ((int32)Layer < ViewLayers.Num())
    {
        return ViewLayers[(int32)Layer]->GetCurrentRoute();
    }

    return FGameplayTag::EmptyTag;
}

const FGameplayTag URHViewManager::GetCurrentTransitionRoute(EViewManagerLayer Layer) const
{
    if ((int32)Layer < ViewLayers.Num())
    {
        return ViewLayers[(int32)Layer]->GetCurrentTransitionRoute();
    }

    return FGameplayTag::EmptyTag;
}

EViewManagerLayer URHViewManager::GetTopLayer() const
{
	for (int32 i = ViewLayers.Num() - 1; i >= 0; i--)
	{
		if (ViewLayers[i])
		{
			// since we assume that layers at higher index are on top,
			// backward iteration means the first layer we find with a CurrentRoute is topmost
			if (ViewLayers[i]->GetCurrentRoute().IsValid())
			{
				return ViewLayers[i]->GetLayerType();
			}
		}
	}
	return EViewManagerLayer::Base; // assumption is this is not reachable
}

const FGameplayTag URHViewManager::GetDefaultRouteForLayer(EViewManagerLayer LayerType) const
{
	for (int32 i = ViewLayers.Num() - 1; i >= 0; i--)
	{
		if (ViewLayers[i])
		{
			if (ViewLayers[i]->GetLayerType() == LayerType)
			{
				return ViewLayers[i]->GetDefaultRoute();
			}
		}
	}
	return FGameplayTag::EmptyTag;
}

bool URHViewManager::HasCompletedRedirectFlow(EViewRouteRedirectionPhase RedirectPhase) const
{
    return ViewRedirectionDisplayCounter[RedirectPhase] == INDEX_NONE;
}

bool URHViewManager::IsBlockingOrders() const
{
	for (auto* Layer : ViewLayers)
	{
		auto* Data = Routes.Find(Layer->GetCurrentRoute());
		if (Data != nullptr && Data->BlockOrders)
		{
			return true;
		}
	}

	return false;
}

bool URHViewManager::IsLayerIdle(EViewManagerLayer LayerType) const
{
	URHViewLayer* ViewLayer = GetLayerByType(LayerType);

	return ViewLayer->GetCurrentTransitionState() == EViewManagerTransitionState::Idle;
}

bool URHViewManager::IsEveryLayerIdle() const
{
	for (URHViewLayer* ViewLayer : ViewLayers)
	{
		if (ViewLayer->GetCurrentTransitionState() != EViewManagerTransitionState::Idle)
		{
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////                                                                                                                                  ////
////                                                                                                                                  ////
////                                                          URHViewLayer                                                         ////
////                                                                                                                                  ////
////                                                                                                                                  ////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void URHViewLayer::Initialize(UCanvasPanel* ViewTarget, URHViewManager* ViewManager, EViewManagerLayer Type)
{
    CurrentTransitionState = EViewManagerTransitionState::Idle;
    DisplayTarget = ViewTarget;
    MyManager = ViewManager;
    LayerType = Type;
}

void URHViewLayer::StoreViewWidget(const FGameplayTag& RouteTag, URHWidget* Widget)
{
    RouteWidgetMap.Add(RouteTag, Widget);

    //$$ DLF - TODO: Figure out why RH was overriding IsFocusable to true here.
#if RH_FROM_ENGINE_VERSION(5,2)
    //Widget->SetIsFocusable(true);
#else
    //Widget->bIsFocusable = true;
#endif
    Widget->InitializeWidget();
    Widget->SetVisibility(ESlateVisibility::Collapsed);

    if (UCanvasPanelSlot* CanvasSlot = DisplayTarget->AddChildToCanvas(Widget))
    {
        CanvasSlot->SetAnchors(FAnchors(0, 0, 1, 1));
        CanvasSlot->SetOffsets(FMargin(0, 0, 0, 0));
    }
}

bool URHViewLayer::ReplaceRoute(const FGameplayTag& RouteTag, const TArray<FGameplayTag>& AdditionalRouteTags, bool ForceTransition/* = false*/, UObject* Data /*= nullptr*/)
{
    if (Data != nullptr)
    {
        PendingRouteData.Add(RouteTag, Data);
    }
    else
    {
        PendingRouteData.Remove(RouteTag);
    }

    TArray<FGameplayTag> RouteTagStack;
	RouteTagStack.Append(AdditionalRouteTags);
	RouteTagStack.Add(RouteTag);

    return RequestRouteStackChange(RouteTagStack, ForceTransition);
}

bool URHViewLayer::PushRoute(const FGameplayTag& RouteTag, const TArray<FGameplayTag>& AdditionalRouteTags, bool ForceTransition/* = false*/, UObject* Data /*= nullptr*/)
{
    if (Data != nullptr)
    {
        PendingRouteData.Add(RouteTag, Data);
    }
    else
    {
        PendingRouteData.Remove(RouteTag);
    }

    // if we're forcing this, we should clear out pending transitions before using the RouteStack
    if (ForceTransition && CurrentTransitionState != EViewManagerTransitionState::Locked)
    {
        ClearPendingTransition();
    }

    TArray<FGameplayTag> RouteTagStack;
    RouteTagStack.Append(CurrentRouteStack);
	RouteTagStack.Append(AdditionalRouteTags);
    RouteTagStack.Add(RouteTag);

    return RequestRouteStackChange(RouteTagStack, ForceTransition);
}

void URHViewLayer::ClearRoutes()
{
	TArray<FGameplayTag> RouteStack;
	RequestRouteStackChange(RouteStack, true);
}

bool URHViewLayer::RemoveRoute(const FGameplayTag& RouteTag, bool ForceTransition/* = false*/)
{
    if (CurrentRouteStack.Num() == 0)
    {
        return false;
    }

    // If we are the top view, pop us
    if (RouteTag == CurrentRouteStack[CurrentRouteStack.Num() - 1])
    {
        return PopRoute();
    }

    bool bFoundFirstNonModal = false;

    // Find ourselves in the route stack to remove
    for (int32 i = 0; i < CurrentRouteStack.Num(); ++i)
    {
        if (RouteTag == CurrentRouteStack[i])
		{
			TArray<FGameplayTag> RouteStack = CurrentRouteStack;
            RouteStack.RemoveAt(i);

			// Allow for the only view to be removed on non-base layers
			if (RouteStack.Num() > 0 || LayerType != EViewManagerLayer::Base)
			{
				return RequestRouteStackChange(RouteStack, ForceTransition);
			}
            return true;
        }
    }

    return false;
}

bool URHViewLayer::PopRoute(bool ForceTransition/* = false*/)
{
    if (CurrentRouteStack.Num() > 0)
    {
        if (ForceTransition && CurrentTransitionState != EViewManagerTransitionState::Locked)
        {
            // do the force transition now so that it can properly pop after the transition finishes.
            ClearPendingTransition();
        }

        TArray<FGameplayTag> RouteStack = CurrentRouteStack;
        RouteStack.Pop();

        // Only allow for the only view to be removed on non-base layers unless specified otherwise by ViewManager
		bool CanBaseLayerBeEmpty = MyManager != nullptr ? MyManager->CanBaseLayerBeEmpty() : false;
        if (RouteStack.Num() > 0|| LayerType != EViewManagerLayer::Base || CanBaseLayerBeEmpty)
        {
            return RequestRouteStackChange(RouteStack, false);
        }
		else if (RouteStack.Num() == 0 && LayerType == EViewManagerLayer::Base)
		{
			// If we are the final route, push our default onto the stack.
			RouteStack.Push(DefaultRoute);
			return RequestRouteStackChange(RouteStack, false);
		}
    }

    return false;
}

bool URHViewLayer::SwapRoute(const FGameplayTag& RouteTag, const FGameplayTag& SwapTargetRoute/* = FGameplayTag::EmptyTag*/, bool ForceTransition/* = false*/)
{
    TArray<FGameplayTag> RouteStack;
    RouteStack.Add(RouteTag);

    bool bShouldAdd = false;
    for (int i = CurrentRouteStack.Num() - 1; i >= 0; i--)
    {
        if (bShouldAdd)
        {
            RouteStack.Insert(CurrentRouteStack[i], 0);
        }

        if (!SwapTargetRoute.IsValid() ||
            CurrentRouteStack[i] == SwapTargetRoute)
        {
            bShouldAdd = true;
        }
    }

    return RequestRouteStackChange(RouteStack, ForceTransition);
}

bool URHViewLayer::ContainsRoute(const FGameplayTag& RouteTag)
{
	return CurrentRouteStack.Contains(RouteTag);
}

bool URHViewLayer::RequestRouteStackChange(const TArray<FGameplayTag>& RouteStack, bool ForceTransition)
{
    // if we're forcing this, we need to tidy up the state
    if (ForceTransition && CurrentTransitionState != EViewManagerTransitionState::Locked)
    {
        // reset any hide/show transition animations
        ClearPendingTransition();
    }

    // If we are going to a clear stack, close out current widget and return
    if (RouteStack.Num() == 0)
	{
		if (CurrentRouteStack.Num() > 0)
		{
			CurrentTransitionRouteStack = RouteStack;

			if (MyManager)
			{
				MyManager->OnViewStateChangeStarted.Broadcast(GetCurrentRoute(), FGameplayTag::EmptyTag, LayerType); // other layers may care
			}

			// Get our old widget and hide it
			if (URHWidget* HidingWidget = *RouteWidgetMap.Find(GetCurrentRoute()))
			{
				if (HidingWidget->IsVisible())
				{
					HidingWidget->OnHideSequenceFinished.AddUniqueDynamic(this, &URHViewLayer::GoToRoute_HandleHideFinished);
					CurrentTransitionState = EViewManagerTransitionState::AnimatingHide;
					HidingWidget->StartHideSequence(GetCurrentRoute(), GetCurrentTransitionRoute());
					HidingWidget->DeactivateWidget(); //$$ JJJT: Addition - CommonUI call
				}
			}

			FinalizeTransition();
		}
        return true;
    }

    bool bFoundValidRoute = RouteStack.Num() > 0;
    for (const FGameplayTag& Route : RouteStack)
    {
        if (!IsRouteValid(Route))
        {
            bFoundValidRoute = false;
            break;
        }
    }

    if (CurrentTransitionState == EViewManagerTransitionState::Idle && bFoundValidRoute && CurrentTransitionRouteStack.Num() <= 0)
    {
        if (GetCurrentRoute() == RouteStack.Top())
        {
            // skip transition, since we're already at the same top route
            CurrentRouteStack = RouteStack;
            return true;
        }

        // Route is looked up in the data table, and found
        FViewRoute* Route = Routes.Find(RouteStack.Top());

        if (MyManager)
        {
            MyManager->OnViewStateChangeStarted.Broadcast(GetCurrentRoute(), RouteStack.Top(), LayerType);
        }

        // TargetRoute is requested(different from current route) - transition state is set to loading
        CurrentTransitionState = EViewManagerTransitionState::Loading;

        URHWidget* TargetRouteWidget = nullptr;

        // The widget is then loaded, ref stored, and added to container where necessary
        if (RouteWidgetMap.Find(RouteStack.Top()) == nullptr && MyManager->GetWorld() != nullptr)
        {
            if (Route->ViewWidget == nullptr)
            {
                CurrentTransitionState = EViewManagerTransitionState::Idle;
                UE_LOG(RallyHereStart, Error, TEXT("URHViewLayer::RequestRouteStackChange Invalid ViewWidget for Route %s"), *RouteStack.Top().ToString());
                return false;
            }

			UE_LOG(RallyHereStart, Log, TEXT("URHViewLayer::RequestRouteStackChange Going to View Route - %s"), *RouteStack.Top().ToString());
            TargetRouteWidget = CreateWidget<URHWidget>(MyManager->GetWorld(), Route->ViewWidget);
            if (TargetRouteWidget != nullptr)
            {
				TargetRouteWidget->SetRouteTag(RouteStack.Top());
                StoreViewWidget(RouteStack.Top(), TargetRouteWidget);
            }
        }

        CurrentTransitionRouteStack = RouteStack;

        if (CurrentRouteStack.Num() <= 0)
        {
            // No previous view widget, so go straight to the show step
            GoToRoute_InternalShowStep();
            return true;
        }
        else
        {
            // Get our old widget and hide it
            if (URHWidget* HidingWidget = *RouteWidgetMap.Find(GetCurrentRoute()))
            {
                if (HidingWidget->IsVisible())
                {
                    HidingWidget->OnHideSequenceFinished.AddUniqueDynamic(this, &URHViewLayer::GoToRoute_HandleHideFinished);
                    CurrentTransitionState = EViewManagerTransitionState::AnimatingHide;
                    HidingWidget->StartHideSequence(GetCurrentRoute(), GetCurrentTransitionRoute());
                    HidingWidget->DeactivateWidget(); //$$ JJJT: Addition - CommonUI call
                    return true;
                }
            }

            GoToRoute_InternalShowStep();
            return true;
        }
    }

    return false;
}

void URHViewLayer::ClearPendingTransition()
{
    if (CurrentTransitionState != EViewManagerTransitionState::Idle && MyManager)
    {
        // hide all of the sticky widgets
        if (LayerType == EViewManagerLayer::Base)
        {
            UE_LOG(RallyHereStart, Verbose, TEXT("ClearPendingTransition:: Hide all sticky widgets"));
            for (auto StickyWidgetKV : MyManager->StickyWidgetMap)
            {
                StickyWidgetKV.Value->HideWidget();
            }
        }

        // stop any hide/reveal that's currently in progress
        URHWidget* TransitioningWidget = nullptr;
        switch (CurrentTransitionState)
        {
        case EViewManagerTransitionState::AnimatingHide:
            TransitioningWidget = *RouteWidgetMap.Find(GetCurrentRoute());
            TransitioningWidget->OnHideSequenceFinished.RemoveDynamic(this, &URHViewLayer::GoToRoute_HandleHideFinished);
            break;
        case EViewManagerTransitionState::AnimatingShow:
            TransitioningWidget = *RouteWidgetMap.Find(GetCurrentTransitionRoute());
            TransitioningWidget->OnShowSequenceFinished.RemoveDynamic(this, &URHViewLayer::GoToRoute_HandleShowFinished);
            break;
        case EViewManagerTransitionState::Loading:
            // fall through, there's nothing to do for a loading widget
        default:
            // do nothing
            break;
        }

        // wish there was a way to stop tick animations tho...
        // TODO!

        // hide the widget that is transitioning
        if (TransitioningWidget != nullptr && TransitioningWidget->IsVisible())
        {
            TransitioningWidget->HideWidget();
        }

		// Get old route before FinalizeTransition wipes it
		FGameplayTag PreviousRoute = GetCurrentRoute();

        FinalizeTransition();

        // broadcast the change
        if (MyManager)
        {
            MyManager->OnViewStateChanged.Broadcast(GetCurrentRoute(), PreviousRoute, LayerType);
        }
    }
}

void URHViewLayer::FinalizeTransition()
{
	bool CanBaseLayerBeEmpty = MyManager != nullptr ? MyManager->CanBaseLayerBeEmpty() : false;
	if (CurrentTransitionRouteStack.Num() > 0 || LayerType != EViewManagerLayer::Base || CanBaseLayerBeEmpty)
	{
		CurrentRouteStack = CurrentTransitionRouteStack;
		CurrentTransitionRouteStack.Empty();
	}

    CurrentTransitionState = EViewManagerTransitionState::Idle;
}

void URHViewLayer::GoToRoute_HandleHideFinished(URHWidget* Widget)
{
    if (Widget)
    {
        Widget->OnHideSequenceFinished.RemoveDynamic(this, &URHViewLayer::GoToRoute_HandleHideFinished);
    }

    // Then move to the next step
    GoToRoute_InternalShowStep();
}

void URHViewLayer::GoToRoute_InternalShowStep()
{
    if (CurrentTransitionRouteStack.Num() == 0)
    {
        if (LayerType != EViewManagerLayer::Base)
        {
			// Get old route before FinalizeTransition wipes it
			FGameplayTag PreviousRoute = GetCurrentRoute();

            FinalizeTransition();

            // Trigger our transition away and finalize it
            if (MyManager)
            {
                MyManager->OnViewStateChanged.Broadcast(GetCurrentRoute(), PreviousRoute, LayerType);
            }
        }

        return;
    }

    if (MyManager && LayerType == EViewManagerLayer::Base)
    {
        FViewRoute* Route = Routes.Find(GetCurrentTransitionRoute());

        // Only manage changing sticky widgets on base screens
        if (Route)
        {
            // We'll handle sticky widgets here, they are handled the same way except the view manager doesn't wait to hear back from them
            for (auto StickyWidgetKV : MyManager->StickyWidgetMap)
            {
                bool bStickyIsShown = StickyWidgetKV.Value->IsVisible();
                bool bStickyIsShownNext = Route->ViewStickyWidgets.Contains(StickyWidgetKV.Key);

                if (bStickyIsShown != bStickyIsShownNext)
                {
                    UE_LOG(RallyHereStart, Verbose, TEXT("Changing Sticky widget visibility for %s for route %s. Current visibilty: %s, should show: %s."), *StickyWidgetKV.Key.ToString(), *GetCurrentTransitionRoute().ToString(), (bStickyIsShown ? TEXT("TRUE") : TEXT("FALSE")), (bStickyIsShownNext ? TEXT("TRUE") : TEXT("FALSE")));
                    if (bStickyIsShown)
                    {
                        // Going from shown to not shown
                        StickyWidgetKV.Value->StartHideSequence(GetCurrentRoute(), GetCurrentTransitionRoute());
                    }
                    else
                    {
                        // Going from not shown to shown
                        StickyWidgetKV.Value->StartShowSequence(GetCurrentRoute(), GetCurrentTransitionRoute());
                    }
                }
                else
                {
                    UE_LOG(RallyHereStart, Verbose, TEXT("Skipping changing Sticky widget visibility for %s for route %s. Current visibilty: %s, should show: %s."), *StickyWidgetKV.Key.ToString(), *GetCurrentTransitionRoute().ToString(), (bStickyIsShown ? TEXT("TRUE") : TEXT("FALSE")), (bStickyIsShownNext ? TEXT("TRUE") : TEXT("FALSE")));
                }
            }
        }
    }

    // Get our next widget and show it
    if (URHWidget* ShowingWidget = *RouteWidgetMap.Find(GetCurrentTransitionRoute()))
    {
        UE_LOG(RallyHereStart, Verbose, TEXT("GoToRoute_InternalShowStep:: Go to screen %s"), *GetCurrentTransitionRoute().ToString());
        ShowingWidget->OnShowSequenceFinished.AddUniqueDynamic(this, &URHViewLayer::GoToRoute_HandleShowFinished);
        CurrentTransitionState = EViewManagerTransitionState::AnimatingShow;
        ShowingWidget->StartShowSequence(GetCurrentRoute(), GetCurrentTransitionRoute());
    }
}

void URHViewLayer::GoToRoute_HandleShowFinished(URHWidget* Widget)
{
    if (Widget)
    {
        Widget->OnShowSequenceFinished.RemoveDynamic(this, &URHViewLayer::GoToRoute_HandleShowFinished);
    }
	
	Widget->ActivateWidget(); //$$ JJJT: Addition - CommonUI call

	// Get old route before FinalizeTransition wipes it
	FGameplayTag PreviousRoute = GetCurrentRoute();

    FinalizeTransition();
 
	if (MyManager)
    {
        MyManager->OnViewStateChanged.Broadcast(GetCurrentRoute(), PreviousRoute, LayerType);

        //set focus to widget with highest priority
        if (ARHHUDCommon* pHUD = MyManager->GetHudRef())
        {
            pHUD->SetUIFocus();
        }
    }
}

bool URHViewLayer::IsRouteValid(FGameplayTag RouteTag)
{
    return Routes.Find(RouteTag) != nullptr;
}

bool URHViewLayer::GetPendingRouteData(const FGameplayTag& RouteTag, UObject*& Data)
{
    UObject** PendingData = PendingRouteData.Find(RouteTag);

    if (PendingData)
    {
        Data = *PendingData;
        return Data != nullptr;
    }

    return false;
}

void URHViewLayer::SetPendingRouteData(const FGameplayTag& RouteTag, UObject* Data)
{
    PendingRouteData.Add(RouteTag, Data);
}