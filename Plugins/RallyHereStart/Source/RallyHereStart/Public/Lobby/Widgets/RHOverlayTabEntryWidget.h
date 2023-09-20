// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHOverlayTabEntryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRHOverlayTabViewEvent, FName, ViewName);

UENUM(BlueprintType)
enum class ERHOverlayTabState : uint8
{
	Idle,				// Default state, not being engaged by anything
	Hovered,			// Highlighted by mouse hover
	Selected,			// Engaged by mouse click or gamepad hover
	SelectedDefocused	// Engaged, but gamepad focus is drilled in instead of here
};

/*
*	Config info for an individual tab in a URHOverlayTabHubBase
*/
USTRUCT(BlueprintType)
struct FOverlayTabViewRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TabName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<URHWidget> ViewWidget;

	// This is an optional validation that determines if a tab should be visible
	UPROPERTY(EditAnywhere)
	TSubclassOf<URHTabValidator> TabValidator;
};

/*
*	Allows a condition to be applied to a tab so that the tab and its view is only present
*	if the validator's function returns true.
*/
UCLASS()
class RALLYHERESTART_API URHTabValidator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual bool IsValidTab() { return true; };
};

/*
 * Tabs associated with a view in a RHOverlayTabHubBase.
 * These controls can tell a RHOverlayTabHubBase to change the active view or move gamepad focus into the view. They can also cosmetically react to those actions.
 */
UCLASS()
class RALLYHERESTART_API URHOverlayTabEntryWidget : public URHWidget
{
	GENERATED_BODY()
	
public:

	virtual void InitializeWidget_Implementation() override;

	virtual void NativeGamepadHover() override;
	virtual void NativeGamepadUnhover() override;

	virtual bool NavigateConfirm_Implementation() override;

	/*
	*	Variables
	*/

	UFUNCTION(BlueprintPure)
	ERHOverlayTabState GetTabState() const { return TabState; };

	UFUNCTION()
	void SetTabState(ERHOverlayTabState InTabState);
	
	UFUNCTION()
	void SetViewName(FName InViewName);

	UFUNCTION(BlueprintPure)
	FOverlayTabViewRow GetViewInfo() const { return MyViewInfo; };

	UFUNCTION()
	void SetViewInfo(FOverlayTabViewRow InViewInfo) ;

protected:

	UPROPERTY()
	ERHOverlayTabState TabState;

	UPROPERTY()
	FName MyViewName;

	UPROPERTY()
	FOverlayTabViewRow MyViewInfo;

	/*
	*	Event handlers
	*/

	UFUNCTION(BlueprintNativeEvent)
	void HandleInputStateChanged(RH_INPUT_STATE InputState);
	virtual void HandleInputStateChanged_Implementation(RH_INPUT_STATE InputState);

public:

	UFUNCTION()
	void HandleActiveViewChanged(FName ActiveView);

	UFUNCTION()
	void HandleViewFocused();

	UFUNCTION()
	void HandleTabsFocused();

protected:

	UFUNCTION()
	void HandleHovered();

	UFUNCTION()
	void HandleUnhovered();

	UFUNCTION()
	void HandleClicked();

	/*
	*	Delegates
	*/

public:

	UPROPERTY(BlueprintAssignable)
	FRHOverlayTabViewEvent OnActiveViewRequested;

	UPROPERTY(BlueprintAssignable)
	FRHOverlayTabViewEvent OnFocusToViewRequested;
	
	/*
	*	Blueprint implementation
	*/

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayTabState();

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayViewInfo();

	UPROPERTY(meta = (BindWidget))
	UButton* HitTarget;
};
