// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RHUIBlueprintFunctionLibrary.h"
#include "Lobby/Widgets/RHOverlayTabHubBase.h"

URHOverlayTabHubBase::URHOverlayTabHubBase(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	TabEntriesFocusGroup = 0;
}

void URHOverlayTabHubBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		CreateAllViews();
	}
}

void URHOverlayTabHubBase::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	CreateAllViews();

	for (URHWidget* ViewWidget : GetViewWidgets())
	{
		ViewWidget->InitializeWidget();
	}

	AddContextActions(TArray<FName>({ FName("Back"), FName("Select") }));

	BackContextAction.BindDynamic(this, &URHOverlayTabHubBase::HandleBackContextAction);
	SetContextAction(FName("Back"), BackContextAction);

	URHUIBlueprintFunctionLibrary::RegisterLinearNavigation(this, GetTabEntries<URHWidget>(), TabEntriesFocusGroup, false, false);
}


void URHOverlayTabHubBase::InitializeWidgetNavigation_Implementation()
{
	Super::InitializeWidgetNavigation_Implementation();

	BindToInputManager(TabEntriesFocusGroup);
}

void URHOverlayTabHubBase::OnShown_Implementation()
{ 
	Super::OnShown_Implementation();

	bool IsDefaultView = false;
	URHWidget* LandingWidget = nullptr;

	if (URHOverlayTabHubRouteData* LandingInfo = GetLandingInfo())
	{
		ChangeView(LandingInfo->RedirectViewName);
		SetFocusToView(LandingInfo->LandingWidgetInView);
	}
	else
	{
		ChangeView(FirstValidViewName);
	}
}

void URHOverlayTabHubBase::OnHide_Implementation()
{
	Super::OnHide_Implementation();

	if (URHWidget* CurrentViewWidget = GetCurrentViewWidget())
	{
		// Possible improvement later: if an overlay view widget gets its own native class too, this can be a specialized function instead
		CurrentViewWidget->HideWidget();
	}
}

bool URHOverlayTabHubBase::NavigateBack_Implementation()
{
	int32 CurrentFocusGroup;
	if (GetCurrentFocusGroup(CurrentFocusGroup) && CurrentFocusGroup == TabEntriesFocusGroup)
	{
		RemoveTopViewRoute();
	}
	else
	{
		SetFocusToTabs();
	}

	return true;
}

void URHOverlayTabHubBase::CreateAllViews()
{
	ViewSwitcher->ClearChildren();
	TabContainer->ClearChildren();
	ViewWidgetMap.Empty();

	FirstValidViewName = FName();

	for (const FName ViewName : ViewNames)
	{
		if (ViewsTable != nullptr)
		{
			FOverlayTabViewRow* ViewRow = ViewsTable->FindRow<FOverlayTabViewRow>(ViewName, TEXT("Finding view info for tab hub"), false);

			if (ViewRow != nullptr && ViewRow->ViewWidget.Get() != nullptr)
			{
				// skip if requirement exists and is not met
				if (ViewRow->TabValidator)
				{
					URHTabValidator* TabValidator = NewObject<URHTabValidator>(this, ViewRow->TabValidator);
					if (!TabValidator->IsValidTab())
					{
						continue;
					}
				}

				AddView(ViewName, *ViewRow);
				if (FirstValidViewName.IsNone())
				{
					FirstValidViewName = ViewName;
				}
			}
		}

	}
}

void URHOverlayTabHubBase::AddView(FName ViewName, FOverlayTabViewRow ViewInfo)
{
	URHOverlayTabEntryWidget* NewTab = nullptr;
	URHWidget* NewViewWidget = nullptr;

	// Add tab
	if (SoftTabWidgetClass.Get() != nullptr)
	{
		NewTab = CreateWidget<URHOverlayTabEntryWidget>(this, SoftTabWidgetClass.Get());
		NewTab->InitializeWidget();
		NewTab->SetViewName(ViewName);
		NewTab->SetViewInfo(ViewInfo);
		NewTab->OnActiveViewRequested.AddDynamic(this, &URHOverlayTabHubBase::HandleActiveViewRequested);
		NewTab->OnFocusToViewRequested.AddDynamic(this, &URHOverlayTabHubBase::HandleFocusToViewRequested);
		TabContainer->AddChild(NewTab);
	}

	// Add screen
	if (ViewInfo.ViewWidget.Get() != nullptr)
	{
		NewViewWidget = CreateWidget<URHWidget>(this, ViewInfo.ViewWidget.Get());
		NewViewWidget->InitializeWidget();
		ViewSwitcher->AddChild(NewViewWidget);
		ViewWidgetMap.Add(ViewName, NewViewWidget);

		// Ensure that OnShown will happen later
		NewViewWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnViewAdded(ViewName, ViewInfo, NewTab, NewViewWidget);
}

void URHOverlayTabHubBase::ChangeView(FName ViewName)
{
	if (URHWidget** FoundViewWidget = ViewWidgetMap.Find(ViewName))
	{
		if (*FoundViewWidget != nullptr)
		{
			if (URHWidget* OldViewWidget = GetCurrentViewWidget())
			{
				// Possible improvement later: if an overlay view widget gets its own native class too, this can be a specialized function instead
				OldViewWidget->HideWidget();
			}

			CurrentViewName = ViewName;

			ViewSwitcher->SetActiveWidget(*FoundViewWidget);

			// Possible improvement later: if an overlay view widget gets its own native class too, this can be a specialized function instead
			(*FoundViewWidget)->ShowWidget();

			for (URHOverlayTabEntryWidget* TabWidget : GetTabEntries())
			{
				TabWidget->HandleActiveViewChanged(ViewName);
				if (TabWidget->GetTabState() == ERHOverlayTabState::Selected && GetCurrentFocusForGroup(TabEntriesFocusGroup) != TabWidget)
				{
					SetFocusToWidgetOfGroup(TabEntriesFocusGroup, TabWidget);
				}
			}

			OnViewChanged();
		}
	}
}

URHWidget* URHOverlayTabHubBase::GetCurrentViewWidget() const
{
	if (URHWidget* const * FoundWidget = ViewWidgetMap.Find(CurrentViewName))
	{
		return *FoundWidget;
	}

	return nullptr;
}

void URHOverlayTabHubBase::SetFocusToTabs()
{
	SetFocusToGroup(TabEntriesFocusGroup, true);

	for (URHOverlayTabEntryWidget* TabWidget : GetTabEntries())
	{
		TabWidget->HandleTabsFocused();
	}

	OnTabsFocused();
}

void URHOverlayTabHubBase::SetFocusToView(URHWidget* InitialFocusedWidget)
{
	if (MyHud.IsValid() && MyHud.Get()->GetCurrentInputState() == PIS_GAMEPAD)
	{
		if (URHWidget** FoundViewWidget = ViewWidgetMap.Find(CurrentViewName))
		{
			int32 ViewGroup = ViewNames.IndexOfByKey(CurrentViewName) + 1;
			InheritFocusGroupFromWidget(ViewGroup, *FoundViewWidget, 0);

			// InheritFocusGroupFromWidget inherits a default focus from the view widget, but this is an option to override it at the hub level, for example if it wants to be pre-navigated to a widget.
			if (InitialFocusedWidget != nullptr)
			{
				SetDefaultFocusForGroup(InitialFocusedWidget, ViewGroup);
			}

			for (URHOverlayTabEntryWidget* TabWidget : GetTabEntries())
			{
				TabWidget->HandleViewFocused();
			}

			SetFocusToGroup(ViewGroup, false);
		}
	}
}

/*
*	Event Handlers
*/

void URHOverlayTabHubBase::HandleBackContextAction()
{
	NavigateBack();
}

void URHOverlayTabHubBase::HandleActiveViewRequested(FName ViewName)
{
	if (CurrentViewName != ViewName)
	{
		ChangeView(ViewName);
	}
}

void URHOverlayTabHubBase::HandleFocusToViewRequested(FName ViewName)
{
	SetFocusToView(nullptr);
}

/*
*	Pending route data
*/

URHOverlayTabHubRouteData* URHOverlayTabHubBase::GetLandingInfo()
{
	// Check for pending route data of the expected type
	UObject* RouteData = nullptr;
	if (GetPendingRouteData(MyRouteName, RouteData))
	{
		if (URHOverlayTabHubRouteData* LandingData = Cast<URHOverlayTabHubRouteData>(RouteData))
		{
			// Go ahead and consume the data now that it's being used
			SetPendingRouteData(MyRouteName, nullptr);

			return LandingData;
		}
	}

	return nullptr;
}

/*
*	Helper functions
*/

TArray<URHOverlayTabEntryWidget*> URHOverlayTabHubBase::GetTabEntries() const
{
	return GetTabEntries<URHOverlayTabEntryWidget>();
};

template<class T>
TArray<T*> URHOverlayTabHubBase::GetTabEntries() const
{
	TArray<T*> ReturnArray = TArray<T*>();

	if (TabContainer != nullptr)
	{
		for (UWidget* ChildWidget : TabContainer->GetAllChildren())
		{
			if (T* ChildWidgetAsType = Cast<T>(ChildWidget))
			{
				ReturnArray.Add(ChildWidgetAsType);
			}
		}
	}

	return ReturnArray;
};

TArray<URHWidget*> URHOverlayTabHubBase::GetViewWidgets() const
{
	TArray<URHWidget*> ViewWidgets;
	ViewWidgetMap.GenerateValueArray(ViewWidgets);
	return ViewWidgets;
};