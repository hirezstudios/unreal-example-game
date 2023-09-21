// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHUIBlueprintFunctionLibrary.h"
#include "Lobby/Widgets/RHOverlayTabEntryWidget.h"
#include "RHOverlayTabHubBase.generated.h"

/*
* Object that contains info that can be passed into this screen via pending route data.
*/
UCLASS(BlueprintType)
class RALLYHERESTART_API URHOverlayTabHubRouteData : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
	FName RedirectViewName;

	// A widget to attempt to gamepad focus upon setting the redirect view.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
	URHWidget* LandingWidgetInView;
};

/**
 * Manages a screen that switches between view widgets using a list of tabs which can themselves be focused and navigable.
 */
UCLASS()
class RALLYHERESTART_API URHOverlayTabHubBase : public URHWidget
{
	GENERATED_BODY()

public:

	URHOverlayTabHubBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void NativePreConstruct() override;
	
	virtual void InitializeWidget_Implementation() override;
	virtual void InitializeWidgetNavigation_Implementation() override;
	virtual void OnShown_Implementation() override;
	virtual void OnHide_Implementation() override;

protected:

	virtual bool NavigateBack_Implementation() override;

	UFUNCTION()
	void CreateAllViews();

	UFUNCTION()
	void AddView(FName ViewName, FOverlayTabViewRow ViewInfo);
	
	// Switches which view is showing, and broadcasts an update to the tabs
	UFUNCTION(Category = "Tab Hub")
	void ChangeView(FName ViewName);

	UFUNCTION(BlueprintPure)
	URHWidget* GetCurrentViewWidget() const;

	// Focuses the tabs and broadcasts an update to the tabs.
	UFUNCTION(Category = "Tab Hub")
	void SetFocusToTabs();
	
	// Focuses the current view and broadcasts an update to the tabs. Can pass in a widget to focus instead of the default
	UFUNCTION(Category = "Tab Hub")
	void SetFocusToView(URHWidget* InitialFocusedWidget);
	
	UPROPERTY(BlueprintReadOnly)
	FName CurrentViewName;

	TMap<FName, URHWidget*> ViewWidgetMap;

	FName FirstValidViewName;
	
	UPROPERTY(BlueprintReadOnly)
	int32 TabEntriesFocusGroup;

	/*
	*	Views config
	*/

	// Table of view definitions to use
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tab Hub")
	UDataTable* ViewsTable;

	// Specific ordered subset of view names
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tab Hub")
	TArray<FName> ViewNames;

	/*
	* Event handlers
	*/

	FOnContextAction BackContextAction;
	
	UFUNCTION()
	void HandleBackContextAction();

	UFUNCTION()
	void HandleActiveViewRequested(FName ViewName);

	UFUNCTION()
	void HandleFocusToViewRequested(FName ViewName);
	
	/*
	*	Pending route data
	*/

	// Consumes and returns saved pending route data for this view route, if it's found
	UFUNCTION()
	URHOverlayTabHubRouteData* GetLandingInfo();

	/*
	*	Helper functions
	*/

	UFUNCTION(BlueprintPure)
	TArray<URHOverlayTabEntryWidget*> GetTabEntries() const;

	// Template helps for native functions that want TArray<URHWidget> or TArray<URHWidget>
	template<class T>
	TArray<T*> GetTabEntries() const;

	UFUNCTION(BlueprintPure)
	TArray<URHWidget*> GetViewWidgets() const;

	/*
	*	Blueprint implementation
	*/
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Hub")
	void OnViewChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Hub")
	void OnTabsFocused();

	// Followup after a view is added to catch anything else that needs to be done to the new widgets, especially for WBP-only stuff
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Hub")
	void OnViewAdded(FName ViewName, FOverlayTabViewRow ViewInfo, URHOverlayTabEntryWidget* TabEntry, URHWidget* ViewWidget);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* ViewSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UPanelWidget* TabContainer;

	UPROPERTY(EditAnywhere)
	TSubclassOf<URHWidget> SoftTabWidgetClass;

};