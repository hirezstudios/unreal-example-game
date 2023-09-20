// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHNavTabWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNavTabSelected, class URHNavTabWidget*, SelectedNavTab);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNavTabUnselected, class URHNavTabWidget*, UnselectedNavTab);

/**
 * Widget for the navigation tabs
 */
UCLASS()
class RALLYHERESTART_API URHNavTabWidget : public URHWidget
{
	GENERATED_BODY()

public:
	URHNavTabWidget(const FObjectInitializer& Initializer);


//Delegates
public:
	//Delegate for when the nav tab is selected
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Tab")
	FOnNavTabSelected OnNavTabSelected;

	//Delegate for when the nav tab is unselected
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Tab")
	FOnNavTabUnselected OnNavTabUnselected;


//Variables
public:
	//If this nav tab is selected right now
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tab")
	bool bSelected;

	//If this nav tab is disabled right now
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tab")
	bool bDisabled;

	//The text on this nav tab button
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab", meta = (ExposeOnSpawn = "true"))
	FText NavText;


//Functions
public:
	//Function for selecting the nav tab
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tab")
	void SelectNavTab();

	//Function for unselecting the nav tab
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tab")
	void UnselectNavTab();

	//Sets the selected status and updates parts of the widget accordingly
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tab")
	void SetSelected(bool bNewSelected);

	//Sets the disabled status and updates parts of the widget accordingly
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tab")
	void SetDisabled(bool bNewDisabled);

	//Is this nav tab button selected right now?
	UFUNCTION(BlueprintPure, Category = "Tab")
	bool IsSelected() const { return bSelected; }

	//Is this nav tab button disabled right now?
	UFUNCTION(BlueprintPure, Category = "Tab")
	bool IsDisabled() const { return bDisabled; }
};
