#include "RallyHereStart.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/Application/NavigationConfig.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "GameFramework/RHPlayerInput.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "RHUIBlueprintFunctionLibrary.h"
#include "Managers/RHInputManager.h"

URHInputManager::URHInputManager(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    if (FSlateApplication::IsInitialized())
    {
		FSlateApplication::Get().SetNavigationConfig(MakeShared<FRHNavigationConfig>());
    }

    NavFailRecursionGate = false;

	OverrideRouteStack = TArray<FName>();
	GlobalRouteName = FName("GlobalContextRoute");
}

void URHInputManager::Initialize(ARHHUDCommon* Hud)
{
	if (ContextActionDataTableClassName.ToString().Len() > 0)
	{
		ContextActionDT = LoadObject<UDataTable>(nullptr, *ContextActionDataTableClassName.ToString(), nullptr, LOAD_None, nullptr);
	}

    MyHud = Hud;
    MyHud->OnInputStateChanged.AddDynamic(this, &URHInputManager::HandleModeChange);
    LastInputState = MyHud->GetCurrentInputState();

    if (APlayerController* PlayerController = MyHud->GetPlayerControllerOwner())
    {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			UInputMappingContext* HUDMappingContext = MyHud->GetHUDMappingContext();
			ContextualMappingContext = NewObject<UInputMappingContext>();

			if (HUDMappingContext != nullptr && ContextualMappingContext != nullptr)
			{
				// Handle different Confirm/Cancel button layouts for different controller hardware
				HUDMappingContext->MapKey(MyHud->GetAcceptPressedAction(), URHPlayerInput::GetGamepadConfirmButton());
				HUDMappingContext->MapKey(MyHud->GetAcceptReleasedAction(), URHPlayerInput::GetGamepadConfirmButton());
				HUDMappingContext->MapKey(MyHud->GetCancelPressedAction(), URHPlayerInput::GetGamepadCancelButton());
				HUDMappingContext->MapKey(MyHud->GetCancelReleasedAction(), URHPlayerInput::GetGamepadCancelButton());
				HUDMappingContext->MapKey(MyHud->GetHoldToConfirmAction(), URHPlayerInput::GetGamepadConfirmButton());
				HUDMappingContext->MapKey(MyHud->GetHoldToCancelAction(), URHPlayerInput::GetGamepadCancelButton());

				if (PlayerController->IsInputComponentInStack(InputComponent))
				{
					PlayerController->PopInputComponent(InputComponent);
				}

				InputComponent = NewObject<UEnhancedInputComponent>(PlayerController);
				BindBaseHUDActions();

				const int32 INPUT_PRIORITY = 5000;
				Subsystem->AddMappingContext(HUDMappingContext, INPUT_PRIORITY);
				Subsystem->AddMappingContext(ContextualMappingContext, INPUT_PRIORITY + 1);
			}
		}
    }
}

void URHInputManager::BindBaseHUDActions()
{
	if (MyHud != nullptr)
	{
		if (APlayerController* PlayerController = MyHud->GetPlayerControllerOwner())
		{
			if (InputComponent != nullptr)
			{
				InputComponent->BindAction(MyHud->GetAcceptPressedAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnAcceptPressed);
				InputComponent->BindAction(MyHud->GetAcceptReleasedAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnAcceptReleased);
				InputComponent->BindAction(MyHud->GetCancelPressedAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnCancelPressed);
				InputComponent->BindAction(MyHud->GetCancelReleasedAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnCancelReleased);
				InputComponent->BindAction(MyHud->GetNavigateUpAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnNavigateUpAction);
				InputComponent->BindAction(MyHud->GetNavigateDownAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnNavigateDownAction);
				InputComponent->BindAction(MyHud->GetNavigateLeftAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnNavigateLeftAction);
				InputComponent->BindAction(MyHud->GetNavigateRightAction(), ETriggerEvent::Triggered, this, &URHInputManager::OnNavigateRightAction);
			}
		}
	}
}

void URHInputManager::HandleModeChange(RH_INPUT_STATE mode)
{
    if (LastFocusedWidget.IsValid())
    {
        ChangeFocus(LastFocusedWidget.Pin().ToSharedRef());
    }
    else if (LastFocusedParentWidget.IsValid())
    {
        TSharedPtr<FRHInputFocusWidget> FocusedWidget;
        if (INDEX_NONE != GetFocusedWidget(LastFocusedParentWidget.Get(), FocusedWidget))
        {
            ChangeFocus(FocusedWidget.ToSharedRef());
        }
    }
}

void URHInputManager::BindParentWidget(URHWidget* ParentWidget, int32 DefaultFocusGroup)
{
    if (InputFocusData.Find(ParentWidget) == nullptr)
    {
        FRHInputFocusDetails newDetails;

        newDetails.DefaultFocusGroupIndex = DefaultFocusGroup;
        newDetails.CurrentFocusGroupIndex = DefaultFocusGroup;

        InputFocusData.Add(ParentWidget, newDetails);
    }
}

void URHInputManager::UnbindParentWidget(URHWidget* ParentWidget)
{
    if (ParentWidget != nullptr)
    {
        InputFocusData.Remove(ParentWidget);
    }
}

void URHInputManager::RegisterWidget(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup, UWidget* Up, UWidget* Down, UWidget* Left, UWidget* Right)
{
    if (TSharedRef<FRHInputFocusWidget>* focusWidget = GetWidget(ParentWidget, Widget, FocusGroup))
    {
        // Widget already exists, update it
        focusWidget->Get().Up = Up;
        focusWidget->Get().Down = Down;
        focusWidget->Get().Left = Left;
        focusWidget->Get().Right = Right;
        return;
    }

    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        TSharedRef<FRHInputFocusWidget> NewWidgetRef(new FRHInputFocusWidget());

        NewWidgetRef->Widget = Widget;
        NewWidgetRef->Up = Up;
        NewWidgetRef->Down = Down;
        NewWidgetRef->Left = Left;
        NewWidgetRef->Right = Right;

        for (int32 i = 0; i < inputDetails->FocusGroups.Num(); i++)
        {
            if (inputDetails->FocusGroups[i].FocusGroupIndex == FocusGroup)
            {
                // Focus group exists, add new widget to the focus group
                inputDetails->FocusGroups[i].Widgets.Add(NewWidgetRef);
                return;
            }
        }

        // First element of a new focus group, create that focus group
        FRHInputFocusGroup newGroup;

        newGroup.FocusGroupIndex = FocusGroup;

        // Set the groups Current and Default Focus to the first widget we pass in for now
        newGroup.Widgets.Push(NewWidgetRef);
        newGroup.DefaultFocusWidget = NewWidgetRef;

        inputDetails->FocusGroups.Add(newGroup);
    }
}

void URHInputManager::UpdateWidgetRegistration(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup, UWidget* Up, UWidget* Down, UWidget* Left, UWidget* Right)
{
	if (TSharedRef<FRHInputFocusWidget>* focusWidget = GetWidget(ParentWidget, Widget, FocusGroup))
	{
		if (Up != nullptr)
		{
			(*focusWidget)->Up = Up;
		}

		if (Down != nullptr)
		{
			(*focusWidget)->Down = Down;
		}

		if (Left != nullptr)
		{
			(*focusWidget)->Left = Left;
		}

		if (Right != nullptr)
		{
			(*focusWidget)->Right = Right;
		}
	}
	else
	{
		RegisterWidget(ParentWidget, Widget, FocusGroup, Up, Down, Left, Right);
	}
}

void URHInputManager::UnregisterWidget(URHWidget* ParentWidget, UWidget* Widget)
{
    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        if (inputDetails->FocusGroups.Num() == 0)
        {
            return;
        }
        //need to make sure the widget is taken out of any focus group it's in
        for (FRHInputFocusGroup& focusGroup : inputDetails->FocusGroups)
        {
            for (int32 nIndex = focusGroup.Widgets.Num() - 1; nIndex >= 0; nIndex--)
            {
                TSharedRef<FRHInputFocusWidget> widget = focusGroup.Widgets[nIndex];
                if (widget->Widget.Get() == Widget)
                {
                    focusGroup.Widgets.RemoveAtSwap(nIndex);
                }
            }
        }
    }
}

void URHInputManager::UnregisterFocusGroup(URHWidget* ParentWidget, int32 FocusGroup)
{
    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        for (int32 i = 0; i < inputDetails->FocusGroups.Num(); i++)
        {
            if (inputDetails->FocusGroups[i].FocusGroupIndex == FocusGroup)
            {
                inputDetails->FocusGroups.RemoveAt(i);
                return;
            }
        }
    }
}

void URHInputManager::InheritFocusGroupFromWidget(URHWidget* TargetWidget, int32 TargetFocusGroupNum, URHWidget* SourceWidget, int32 SourceFocusGroupNum)
{
	FRHInputFocusDetails* targetInputDetails = InputFocusData.Find(TargetWidget);
	FRHInputFocusDetails* sourceInputDetails = InputFocusData.Find(SourceWidget);

	if (targetInputDetails && sourceInputDetails)
	{
		int32 SourceFocusGroupIndex = INDEX_NONE;
		int32 TargetFocusGroupIndex = INDEX_NONE;

		for (int32 i = 0; i < sourceInputDetails->FocusGroups.Num(); i++)
		{
			if (sourceInputDetails->FocusGroups[i].FocusGroupIndex == SourceFocusGroupNum)
			{
				SourceFocusGroupIndex = i;
				break;
			}
		}

		if (SourceFocusGroupIndex != INDEX_NONE)
		{
			for (int32 i = 0; i < targetInputDetails->FocusGroups.Num(); i++)
			{
				if (targetInputDetails->FocusGroups[i].FocusGroupIndex == TargetFocusGroupNum)
				{
					TargetFocusGroupIndex = i;
					break;
				}
			}
		}

		if (TargetFocusGroupIndex == INDEX_NONE)
		{
			// Create a new group, instead as a copy of source
			FRHInputFocusGroup newGroup;

			newGroup.FocusGroupIndex = TargetFocusGroupNum;
			newGroup.Widgets.Append(sourceInputDetails->FocusGroups[SourceFocusGroupIndex].Widgets);
			newGroup.DefaultFocusWidget = sourceInputDetails->FocusGroups[SourceFocusGroupIndex].DefaultFocusWidget;
			newGroup.CurrentFocusWidget = sourceInputDetails->FocusGroups[SourceFocusGroupIndex].CurrentFocusWidget;

			targetInputDetails->FocusGroups.Add(newGroup);
		}
		else
		{
			// Copy details of other focus group over this one
			targetInputDetails->FocusGroups[TargetFocusGroupIndex].Widgets.Empty(sourceInputDetails->FocusGroups[SourceFocusGroupIndex].Widgets.Num());
			targetInputDetails->FocusGroups[TargetFocusGroupIndex].Widgets.Append(sourceInputDetails->FocusGroups[SourceFocusGroupIndex].Widgets);
			targetInputDetails->FocusGroups[TargetFocusGroupIndex].DefaultFocusWidget = sourceInputDetails->FocusGroups[SourceFocusGroupIndex].DefaultFocusWidget;
			targetInputDetails->FocusGroups[TargetFocusGroupIndex].CurrentFocusWidget = sourceInputDetails->FocusGroups[SourceFocusGroupIndex].CurrentFocusWidget;
		}
	}
}

void URHInputManager::SetFocusToGroup(URHWidget* ParentWidget, int32 FocusGroup, bool KeepLastFocus)
{
    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        inputDetails->CurrentFocusGroupIndex = FocusGroup;

        for (int32 i = 0; i < inputDetails->FocusGroups.Num(); i++)
        {
            if (inputDetails->FocusGroups[i].FocusGroupIndex == FocusGroup)
            {
                if (!KeepLastFocus || inputDetails->FocusGroups[i].CurrentFocusWidget == nullptr)
                {
					// Grab our default widget
					TWeakPtr<FRHInputFocusWidget> defaultWidget = inputDetails->FocusGroups[i].DefaultFocusWidget;
					// Do a full sweep to find first available enabled widget if default is bad or disabled
					if (defaultWidget.IsValid() && defaultWidget.Pin().Get()->Widget.IsValid() && !defaultWidget.Pin().Get()->Widget.Get()->GetIsEnabled())
					{
						for (int32 j = 0; j < inputDetails->FocusGroups[i].Widgets.Num(); j++)
						{
							defaultWidget = GetWidget(ParentWidget, inputDetails->FocusGroups[i].Widgets[j]->Widget.Get(), FocusGroup)->Get().AsShared();
							if (defaultWidget.IsValid() && defaultWidget.Pin().Get()->Widget.Get()->GetIsEnabled())
							{
								// break with the first enabled widget we find
								break;
							}
						}
					}
					if (!(defaultWidget.IsValid() && defaultWidget.Pin().Get()->Widget.IsValid() && defaultWidget.Pin().Get()->Widget.Get()->GetIsEnabled()))
					{
						// everything is disabled, throwing a warning
						UE_LOG(RallyHereStart, Warning, TEXT("Unable to find enabled default widget in focus group %d in Parent Widget %s, using disabled default widget"), FocusGroup, *ParentWidget->GetName());
						defaultWidget = inputDetails->FocusGroups[i].DefaultFocusWidget;
					}

                    inputDetails->FocusGroups[i].CurrentFocusWidget = defaultWidget;
                }

                if (ParentWidget == LastFocusedParentWidget.Get())
                {
                    if (inputDetails->FocusGroups[i].CurrentFocusWidget.IsValid() && inputDetails->FocusGroups[i].CurrentFocusWidget.Pin().IsValid())
                    {
                        ChangeFocus(inputDetails->FocusGroups[i].CurrentFocusWidget.Pin().ToSharedRef());
                    }
                    else if (inputDetails->FocusGroups[i].DefaultFocusWidget.IsValid() && inputDetails->FocusGroups[i].DefaultFocusWidget.Pin().IsValid())
                    {
                        ChangeFocus(inputDetails->FocusGroups[i].DefaultFocusWidget.Pin().ToSharedRef());
                    }
                }
                break;
            }
        }
    }
}

void URHInputManager::SetFocusToWidgetOfGroup(URHWidget* ParentWidget, int32 FocusGroup, URHWidget* Widget)
{
    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        inputDetails->CurrentFocusGroupIndex = FocusGroup;

        for (int32 i = 0; i < inputDetails->FocusGroups.Num(); ++i)
        {
            if (inputDetails->FocusGroups[i].FocusGroupIndex == FocusGroup)
            {
                for (int32 j = 0; j < inputDetails->FocusGroups[i].Widgets.Num(); ++j)
                {
                    if (inputDetails->FocusGroups[i].Widgets[j]->Widget.IsValid() && inputDetails->FocusGroups[i].Widgets[j]->Widget.Get() == Widget)
                    {
                        inputDetails->FocusGroups[i].CurrentFocusWidget = inputDetails->FocusGroups[i].Widgets[j];

                        if (inputDetails->FocusGroups[i].CurrentFocusWidget.IsValid() && inputDetails->FocusGroups[i].CurrentFocusWidget.Pin().IsValid())
                        {
                            ChangeFocus(inputDetails->FocusGroups[i].CurrentFocusWidget.Pin().ToSharedRef());
                        }

                        break;
                    }
                }
                
                break;
            }
        }
    }
}

TSharedRef<FRHInputFocusWidget>* URHInputManager::GetWidget(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup)
{
    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        for (int32 i = 0; i < inputDetails->FocusGroups.Num(); i++)
        {
            if (inputDetails->FocusGroups[i].FocusGroupIndex == FocusGroup)
            {
                for (int32 j = 0; j < inputDetails->FocusGroups[i].Widgets.Num(); j++)
                {
                    if (inputDetails->FocusGroups[i].Widgets[j]->Widget.IsValid() && inputDetails->FocusGroups[i].Widgets[j]->Widget == Widget)
                    {
                        return &inputDetails->FocusGroups[i].Widgets[j];
                    }
                }

                return nullptr;
            }
        }
    }

    return nullptr;
}

bool URHInputManager::GetFocusedWidget(URHWidget* ParentWidget, UWidget*& FocusWidget)
{
    TSharedPtr<FRHInputFocusWidget> focusedWidget;

    GetFocusedWidget(ParentWidget, focusedWidget);

    if (focusedWidget.IsValid() && focusedWidget.Get()->Widget.IsValid())
    {
        FocusWidget = focusedWidget.Get()->Widget.Get();
        return true;
    }

    return false;
}

int32 URHInputManager::GetFocusedWidget(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget>& FocusWidget)
{
    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        FRHInputFocusGroup* defaultFocusGroup = nullptr;
        FRHInputFocusGroup* currentFocusGroup = nullptr;
        int32 currentFocusIndex = INDEX_NONE;
        int32 defaultFocusIndex = INDEX_NONE;

        for (int32 i = 0; i < inputDetails->FocusGroups.Num(); i++)
        {
            if (inputDetails->FocusGroups[i].FocusGroupIndex == inputDetails->CurrentFocusGroupIndex)
            {
                currentFocusGroup = &inputDetails->FocusGroups[i];
                currentFocusIndex = i;
            }

            if (inputDetails->FocusGroups[i].FocusGroupIndex == inputDetails->DefaultFocusGroupIndex)
            {
                defaultFocusGroup = &inputDetails->FocusGroups[i];
                defaultFocusIndex = i;
            }
        }

        if (currentFocusGroup != nullptr)
        {
            if (currentFocusGroup->CurrentFocusWidget.IsValid() && currentFocusGroup->CurrentFocusWidget.Pin()->Widget.IsValid())
            {
                FocusWidget = currentFocusGroup->CurrentFocusWidget.Pin();
                return currentFocusIndex;
            }
            else if (currentFocusGroup->DefaultFocusWidget.IsValid() && currentFocusGroup->DefaultFocusWidget.Pin()->Widget.IsValid())
            {
                FocusWidget = currentFocusGroup->DefaultFocusWidget.Pin();
                return currentFocusIndex;
            }
        }

        if (defaultFocusGroup != nullptr)
        {
            if (defaultFocusGroup->CurrentFocusWidget.IsValid() && defaultFocusGroup->CurrentFocusWidget.Pin()->Widget.IsValid())
            {
                FocusWidget = defaultFocusGroup->CurrentFocusWidget.Pin();
                return defaultFocusIndex;
            }
            else if (defaultFocusGroup->DefaultFocusWidget.IsValid() && defaultFocusGroup->DefaultFocusWidget.Pin()->Widget.IsValid())
            {
                FocusWidget = defaultFocusGroup->DefaultFocusWidget.Pin();
                return defaultFocusIndex;
            }
        }
    }

    return INDEX_NONE;
}

UWidget* URHInputManager::GetCurrentFocusForGroup(URHWidget* ParentWidget, int32 FocusGroup)
{
    if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
    {
        for (int32 i = 0; i < inputDetails->FocusGroups.Num(); i++)
        {
            if (inputDetails->FocusGroups[i].FocusGroupIndex == FocusGroup)
            {
                if (inputDetails->FocusGroups[i].CurrentFocusWidget.IsValid())
                {
                    return inputDetails->FocusGroups[i].CurrentFocusWidget.Pin()->Widget.Get();
                }
                else
                {
                    return nullptr;
                }
            }
        }
    }

    return nullptr;
}

bool URHInputManager::GetCurrentFocusGroup(URHWidget* ParentWidget, int32& OutFocusGroup)
{
	if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
	{
		OutFocusGroup = inputDetails->CurrentFocusGroupIndex;
		return true;
	}

	return false;
}

void URHInputManager::SetDefaultFocusForGroup(URHWidget* ParentWidget, UWidget* Widget, int32 FocusGroup)
{
    TSharedRef<FRHInputFocusWidget>* targetWidget = GetWidget(ParentWidget, Widget, FocusGroup);

    if (targetWidget)
    {
        if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
        {
            for (int32 i = 0; i < inputDetails->FocusGroups.Num(); i++)
            {
                if (inputDetails->FocusGroups[i].FocusGroupIndex == FocusGroup)
                {
                    inputDetails->FocusGroups[i].DefaultFocusWidget = targetWidget->Get().AsShared();
                    if (!inputDetails->FocusGroups[i].CurrentFocusWidget.IsValid())
                    {
                        inputDetails->FocusGroups[i].CurrentFocusWidget = inputDetails->FocusGroups[i].DefaultFocusWidget;
                    }
                }
            }
        }
    }
}

UWidget* URHInputManager::SetFocusToThis(URHWidget* ParentWidget)
{
    TSharedPtr<FRHInputFocusWidget> focusedWidget;
    int32 groupIndex = GetFocusedWidget(ParentWidget, focusedWidget);

    if (groupIndex != INDEX_NONE && LastFocusedParentWidget != ParentWidget)
    {
        LastFocusedParentWidget = ParentWidget;
        return ChangeFocus(focusedWidget->AsShared());
    }
    else
    {
        UE_LOG(RallyHereStart, Error, TEXT("Failed to find focused widget to transfer to."));
    }

    return nullptr;
}

UWidget* URHInputManager::ChangeFocus(TSharedRef<FRHInputFocusWidget> NewFocus)
{
    if (MyHud.IsValid())
    {
        if (LastFocusedWidget.IsValid())
        {
            TSharedPtr<FRHInputFocusWidget> pinnedLast = LastFocusedWidget.Pin();
            if (pinnedLast.IsValid() && pinnedLast->Widget.IsValid())
            {
                URHWidget* RHWidget = Cast<URHWidget>(pinnedLast->Widget.Get());
                if (RHWidget != NewFocus->Widget || !IsUIFocused())
                {
                    if (RHWidget != nullptr)
                    {
                        if (MyHud->GetCurrentInputState() == RH_INPUT_STATE::PIS_GAMEPAD)
                        {
                            RHWidget->OnGamepadUnhover();
                        }
                        else
                        {
                            RHWidget->TakeWidget().Get().OnMouseLeave(FPointerEvent());
                        }

                        RHWidget->ClearPressedStates();
                    }
                    else if (RHWidget == nullptr)
                    {
                        // fallback mouse behaviour for all non-RH widgets
                        LastFocusedWidget.Pin().Get()->Widget->TakeWidget().Get().OnMouseLeave(FPointerEvent());
                    }
                }
            }
        }

        UWidget* widget = nullptr;

        if (NewFocus->Widget.IsValid())
        {
            widget = NewFocus->Widget.Get();
        }

        if (widget != nullptr)
        {
            LastFocusedWidget = NewFocus;
            if (LastFocusedParentWidget.IsValid() && IsUIFocused())
            {
                widget->SetKeyboardFocus();
                URHWidget* RHWidget = Cast<URHWidget>(widget);
                if (RHWidget != nullptr)
                {
                    if (MyHud->GetCurrentInputState() == RH_INPUT_STATE::PIS_GAMEPAD)
                    {
                        RHWidget->OnGamepadHover();
                    }
                    else
                    {
                        RHWidget->TakeWidget().Get().OnMouseEnter(FGeometry(), FPointerEvent());
                    }
                }
                else
                {
					// fallback mouse behaviour for all non-RH widgets
                    widget->TakeWidget().Get().OnMouseEnter(FGeometry(), FPointerEvent());
                }

                return widget;
            }
        }
    }

    return nullptr;
}

bool URHInputManager::IsUIFocused()
{
    UE_LOG(RallyHereStart, Log, TEXT("Override IsUIFocused() with your game's logic for determining UI focus."));
    return true;
}

void URHInputManager::NavigateUp()
{
	if (this->IsNavInputThrottled(ERHLastInputType::ERHLastInputType_Up))
	{
		return;
	}
	else
	{
		// throttle further input
		this->SetNavInputThrottled(ERHLastInputType::ERHLastInputType_Up);
	}

    if (!MyHud.IsValid())
    {
        return;
    }

    URHWidget* ParentWidget = nullptr;
    if (LastFocusedParentWidget.IsValid())
    {
        ParentWidget = LastFocusedParentWidget.Get();
    }
    else
    {
        return;
    }

    TSharedPtr<FRHInputFocusWidget> focusedWidget;
    int32 groupIndex = GetFocusedWidget(ParentWidget, focusedWidget);

    if (groupIndex != INDEX_NONE)
    {
        if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
        {
            FRHInputFocusGroup* focusedGroup = &inputDetails->FocusGroups[groupIndex];

            NavigateUp_Internal(ParentWidget, focusedWidget, focusedGroup);
        }
    }

    return;
}

bool URHInputManager::NavigateUp_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup)
{
    if (FocusedWidget->Up.IsValid())
    {
        const TSharedRef<FRHInputFocusWidget>* targetWidget = GetWidget(ParentWidget, FocusedWidget->Up.Get(), FocusedGroup->FocusGroupIndex);

        while (targetWidget != nullptr)
        {
            const UWidget* widget = nullptr;

            if (targetWidget->Get().Widget.IsValid())
            {
                widget = targetWidget->Get().Widget.Get();
            }

            if (widget != nullptr && widget->IsVisible() && widget->GetIsEnabled())
            {
                FocusedGroup->CurrentFocusWidget = targetWidget->Get().AsShared();
				ChangeFocus(targetWidget->Get().AsShared());
                return true;
            }

            // Find next up
            targetWidget = GetWidget(ParentWidget, targetWidget->Get().Up.Get(), FocusedGroup->FocusGroupIndex);
        }
    }

    if (!NavFailRecursionGate)
    {
        NavFailRecursionGate = true;

		// Failure event on the individual focused RHWidget 
        URHWidget* RHWidget = Cast<URHWidget>(FocusedWidget->Widget.Get());
        if (RHWidget != nullptr)
        {
			RHWidget->NavigateUpFailure();
        }

		// Also, a similar failure event, associated with the current focus group rather than the widget.
		ParentWidget->NativeFocusGroupNavigateUpFailure(FocusedGroup->FocusGroupIndex, RHWidget);

        NavFailRecursionGate = false;
    }

    return false;
}

void URHInputManager::NavigateDown()
{
	if (this->IsNavInputThrottled(ERHLastInputType::ERHLastInputType_Down))
	{
		return;
	}
	else
	{
		// throttle further input
		this->SetNavInputThrottled(ERHLastInputType::ERHLastInputType_Down);
	}

    if (!MyHud.IsValid())
    {
        return;
    }

    URHWidget* ParentWidget = nullptr;
    if (LastFocusedParentWidget.IsValid())
    {
        ParentWidget = LastFocusedParentWidget.Get();
    }
    else
    {
        return;
    }

    TSharedPtr<FRHInputFocusWidget> focusedWidget;
    int32 groupIndex = GetFocusedWidget(ParentWidget, focusedWidget);

    if (groupIndex != INDEX_NONE)
    {
        if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
        {
            FRHInputFocusGroup* focusedGroup = &inputDetails->FocusGroups[groupIndex];

            NavigateDown_Internal(ParentWidget, focusedWidget, focusedGroup);
        }
    }

    return;
}

bool URHInputManager::NavigateDown_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup)
{
    if (FocusedWidget->Down.IsValid())
    {
        const TSharedRef<FRHInputFocusWidget>* targetWidget = GetWidget(ParentWidget, FocusedWidget->Down.Get(), FocusedGroup->FocusGroupIndex);

        while (targetWidget != nullptr)
        {
            const UWidget* widget = nullptr;

            if (targetWidget->Get().Widget.IsValid())
            {
                widget = targetWidget->Get().Widget.Get();
            }

            if (widget != nullptr && widget->IsVisible() && widget->GetIsEnabled())
            {
                FocusedGroup->CurrentFocusWidget = targetWidget->Get().AsShared();
                ChangeFocus(targetWidget->Get().AsShared());
                return true;
            }

            // Find next down
            targetWidget = GetWidget(ParentWidget, targetWidget->Get().Down.Get(), FocusedGroup->FocusGroupIndex);
        }
    }

    if (!NavFailRecursionGate)
    {
		NavFailRecursionGate = true;

		// Failure event on the individual focused RHWidget  
		URHWidget* RHWidget = Cast<URHWidget>(FocusedWidget->Widget.Get());
		if (RHWidget != nullptr)
		{
			RHWidget->NavigateDownFailure();
		}

		// Also, a similar failure event, associated with the current focus group rather than the widget.
		ParentWidget->NativeFocusGroupNavigateDownFailure(FocusedGroup->FocusGroupIndex, RHWidget);

		NavFailRecursionGate = false;
    }

    return false;
}

void URHInputManager::NavigateLeft()
{
	if (this->IsNavInputThrottled(ERHLastInputType::ERHLastInputType_Left))
	{
		return;
	}
	else
	{
		// throttle further input
		this->SetNavInputThrottled(ERHLastInputType::ERHLastInputType_Left);
	}

    if (!MyHud.IsValid())
    {
        return;
    }

    URHWidget* ParentWidget = nullptr;
    if (LastFocusedParentWidget.IsValid())
    {
        ParentWidget = LastFocusedParentWidget.Get();
    }
    else
    {
        return;
    }

    TSharedPtr<FRHInputFocusWidget> focusedWidget;
    int32 groupIndex = GetFocusedWidget(ParentWidget, focusedWidget);

    if (groupIndex != INDEX_NONE)
    {
        if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
        {
            FRHInputFocusGroup* focusedGroup = &inputDetails->FocusGroups[groupIndex];

            NavigateLeft_Internal(ParentWidget, focusedWidget, focusedGroup);
        }
    }

    return;
}

bool URHInputManager::NavigateLeft_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup)
{
    if (FocusedWidget->Left.IsValid())
    {
        const TSharedRef<FRHInputFocusWidget>* targetWidget = GetWidget(ParentWidget, FocusedWidget->Left.Get(), FocusedGroup->FocusGroupIndex);

        while (targetWidget != nullptr)
        {
            const UWidget* widget = nullptr;

            if (targetWidget->Get().Widget.IsValid())
            {
                widget = targetWidget->Get().Widget.Get();
            }
        	
            const bool hasVisibility = widget->GetVisibility() == ESlateVisibility::HitTestInvisible || widget->GetVisibility() == ESlateVisibility::Visible || widget->GetVisibility() == ESlateVisibility::SelfHitTestInvisible;
            if (widget != nullptr && (widget->IsVisible() || hasVisibility) && widget->GetIsEnabled())
            {
                FocusedGroup->CurrentFocusWidget = targetWidget->Get().AsShared();
				ChangeFocus(targetWidget->Get().AsShared());
                UE_LOG(RallyHereStart, Warning, TEXT("NAVIGATE LEFT SUCCESS."));
                return true;
            }
            UE_LOG(RallyHereStart, Warning, TEXT("NAVIGATE LEFT FAILED."));

            // Find next left
            targetWidget = GetWidget(ParentWidget, targetWidget->Get().Left.Get(), FocusedGroup->FocusGroupIndex);
        }
    }

    if (!NavFailRecursionGate)
    {
		NavFailRecursionGate = true;

		// Failure event on the individual focused RHWidget  
		URHWidget* RHWidget = Cast<URHWidget>(FocusedWidget->Widget.Get());
		if (RHWidget != nullptr)
		{
			RHWidget->NavigateLeftFailure();
		}

		// Also, a similar failure event, associated with the current focus group rather than the widget.
		ParentWidget->NativeFocusGroupNavigateLeftFailure(FocusedGroup->FocusGroupIndex, RHWidget);

		NavFailRecursionGate = false;
    }

    return false;
}

void URHInputManager::NavigateRight()
{
	if (this->IsNavInputThrottled(ERHLastInputType::ERHLastInputType_Right))
	{
		return;
	}
	else
	{
		// throttle further input
		this->SetNavInputThrottled(ERHLastInputType::ERHLastInputType_Left);
	}

    if (!MyHud.IsValid())
    {
        return;
    }

    URHWidget* ParentWidget = nullptr;
    if (LastFocusedParentWidget.IsValid())
    {
        ParentWidget = LastFocusedParentWidget.Get();
    }
    else
    {
        return;
    }

    TSharedPtr<FRHInputFocusWidget> focusedWidget;
    int32 groupIndex = GetFocusedWidget(ParentWidget, focusedWidget);

    if (groupIndex != INDEX_NONE)
    {
        if (FRHInputFocusDetails* inputDetails = InputFocusData.Find(ParentWidget))
        {
            FRHInputFocusGroup* focusedGroup = &inputDetails->FocusGroups[groupIndex];

            NavigateRight_Internal(ParentWidget, focusedWidget, focusedGroup);
        }
    }

    return;
}

bool URHInputManager::NavigateRight_Internal(URHWidget* ParentWidget, TSharedPtr<FRHInputFocusWidget> FocusedWidget, FRHInputFocusGroup* FocusedGroup)
{
    if (FocusedWidget->Right.IsValid())
    {
        const TSharedRef<FRHInputFocusWidget>* targetWidget = GetWidget(ParentWidget, FocusedWidget->Right.Get(), FocusedGroup->FocusGroupIndex);

        while (targetWidget != nullptr)
        {
            const UWidget* widget = nullptr;

            if (targetWidget->Get().Widget.IsValid())
            {
                widget = targetWidget->Get().Widget.Get();
            }

            if (widget != nullptr && widget->IsVisible() && widget->GetIsEnabled())
            {
                FocusedGroup->CurrentFocusWidget = targetWidget->Get().AsShared();
				ChangeFocus(targetWidget->Get().AsShared());
                UE_LOG(RallyHereStart, Warning, TEXT("NAVIGATE RIGHT SUCCESS."));
                return true;
            }
            UE_LOG(RallyHereStart, Warning, TEXT("NAVIGATE RIGHT FAILED."));

            // Find next right
            targetWidget = GetWidget(ParentWidget, targetWidget->Get().Right.Get(), FocusedGroup->FocusGroupIndex);
        }
    }

    if (!NavFailRecursionGate)
    {
		NavFailRecursionGate = true;

		// Failure event on the individual focused RHWidget  
		URHWidget* RHWidget = Cast<URHWidget>(FocusedWidget->Widget.Get());
		if (RHWidget != nullptr)
		{
			RHWidget->NavigateRightFailure();
		}

		// Also, a similar failure event, associated with the current focus group rather than the widget.
		ParentWidget->NativeFocusGroupNavigateRightFailure(FocusedGroup->FocusGroupIndex, RHWidget);

		NavFailRecursionGate = false;
    }

    return false;
}

void URHInputManager::OnAcceptPressed(const FInputActionValue& Value)
{
	TSharedPtr<FRHInputFocusWidget> focusedWidget;
	int32 groupIndex = GetFocusedWidget(LastFocusedParentWidget.Get(), focusedWidget);

	InputLockWidget = focusedWidget;

	if (groupIndex != INDEX_NONE)
	{
		URHWidget* RHWidget = Cast<URHWidget>(focusedWidget->Widget.Get());
		if (RHWidget != nullptr)
		{
			if (RHWidget->NavigateConfirmPressedOnWidget())
			{
				return;
			}
		}
		LastFocusedParentWidget->NavigateConfirmPressedOnWidget();
	}
	else
	{
		UE_LOG(RallyHereStart, Error, TEXT("No valid focused widget!"));
	}
}

void URHInputManager::OnAcceptReleased(const FInputActionValue& Value)
{
    TSharedPtr<FRHInputFocusWidget> focusedWidget;
    int32 groupIndex = GetFocusedWidget(LastFocusedParentWidget.Get(), focusedWidget);

	if (InputLockWidget.IsValid() && focusedWidget != InputLockWidget)
	{
		UE_LOG(RallyHereStart, Warning, TEXT("Skipping Accept / Cancel release - the press was on a different widget."))
		// We can, however, let the input lock widget know in case it has to handle that its press action was canceled.
		URHWidget* InputLockRHWidget = Cast<URHWidget>(InputLockWidget->Widget.Get());
		if (InputLockRHWidget != nullptr)
		{
			InputLockRHWidget->NavigateConfirmCancelledOnWidget();
		}
		InputLockWidget.Reset();
		return;
	}

	InputLockWidget.Reset();

    if (groupIndex != INDEX_NONE)
    {
        URHWidget* RHWidget = Cast<URHWidget>(focusedWidget->Widget.Get());
		if (RHWidget != nullptr)
		{
			if (RHWidget->NavigateConfirmOnWidget())
			{
				return;
			}
		}
		LastFocusedParentWidget->NavigateConfirmOnWidget();
    }
    else
    {
        UE_LOG(RallyHereStart, Error, TEXT("No valid focused widget!"));
    }
}

void URHInputManager::OnCancelPressed(const FInputActionValue& Value)
{
	TSharedPtr<FRHInputFocusWidget> focusedWidget;
	int32 groupIndex = GetFocusedWidget(LastFocusedParentWidget.Get(), focusedWidget);

	InputLockWidget = focusedWidget;

	if (groupIndex != INDEX_NONE)
	{
		URHWidget* RHWidget = Cast<URHWidget>(focusedWidget->Widget.Get());
		if (RHWidget != nullptr)
		{
			if (RHWidget->NavigateBackPressedOnWidget())
			{
				return;
			}
		}
		LastFocusedParentWidget->NavigateBackPressedOnWidget();
	}
	else
	{
		UE_LOG(RallyHereStart, Error, TEXT("No valid focused widget!"));
	}
}

void URHInputManager::OnCancelReleased(const FInputActionValue& Value)
{
	TSharedPtr<FRHInputFocusWidget> focusedWidget;
	int32 groupIndex = GetFocusedWidget(LastFocusedParentWidget.Get(), focusedWidget);

	if (InputLockWidget.IsValid() && focusedWidget != InputLockWidget)
	{
		UE_LOG(RallyHereStart, Warning, TEXT("Skipping Accept / Cancel release - the press was on a different widget."))
		// We can, however, let the input lock widget know in case it has to handle that its press action was canceled.
		URHWidget* InputLockRHWidget = Cast<URHWidget>(InputLockWidget->Widget.Get());
		if (InputLockRHWidget != nullptr)
		{
			InputLockRHWidget->NavigateBackCancelledOnWidget();
		}
		InputLockWidget.Reset();
		return;
	}

	InputLockWidget.Reset();

	if (groupIndex != INDEX_NONE)
	{
		URHWidget* RHWidget = Cast<URHWidget>(focusedWidget->Widget.Get());
		if (RHWidget != nullptr)
		{
			if (RHWidget->NavigateBackOnWidget())
			{
				return;
			}
		}
		LastFocusedParentWidget->NavigateBackOnWidget();
	}
	else
	{
		UE_LOG(RallyHereStart, Error, TEXT("No valid focused widget!"));
	}
}

void URHInputManager::SetNavigationEnabled(bool enabled)
{
    if (MyHud != nullptr)
    {
        if (enabled)
        {
            MyHud->GetPlayerControllerOwner()->PushInputComponent(InputComponent);
        }
        else
        {
            MyHud->GetPlayerControllerOwner()->PopInputComponent(InputComponent);
        }
    }
}

void URHInputManager::SetNavInputThrottled(ERHLastInputType InputType)
{
    static float TimeToLockInput = 0.08f;
    static float TimeToLockDebounceInput = 0.75f;
    if (!this->IsNavInputThrottled(InputType))
    {
        LastInputType = InputType;
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();

        // Clear out throttle from other inputs
        if (TimerManager.IsTimerActive(NavInputThrottleTimerHandle))
        {
            ClearNavInputDebouncedThrottled();
        }
        TimerManager.SetTimer(NavInputDebounceThrottleTimerHandle, this, &URHInputManager::ClearNavInputDebouncedThrottled, TimeToLockDebounceInput, false);
        TimerManager.SetTimer(NavInputThrottleTimerHandle, this, &URHInputManager::ClearNavInputThrottled, TimeToLockInput, false);
    }
}

bool URHInputManager::IsNavInputDebounceThrottled(ERHLastInputType InputType)
{
    if (GetWorld()->GetTimerManager().IsTimerActive(NavInputThrottleTimerHandle))
    {
        switch (InputType)
        {
        case ERHLastInputType::ERHLastInputType_Up:
            return InputType == ERHLastInputType::ERHLastInputType_Down;
        case ERHLastInputType::ERHLastInputType_Down:
            return InputType == ERHLastInputType::ERHLastInputType_Up;
        case ERHLastInputType::ERHLastInputType_Left:
            return InputType == ERHLastInputType::ERHLastInputType_Right;
        case ERHLastInputType::ERHLastInputType_Right:
            return InputType == ERHLastInputType::ERHLastInputType_Left;
        }
    }

    return false;
}

void URHInputManager::ClearNavInputDebouncedThrottled()
{
    GetWorld()->GetTimerManager().ClearTimer(NavInputDebounceThrottleTimerHandle);
}

void URHInputManager::ClearNavInputThrottled()
{
    GetWorld()->GetTimerManager().ClearTimer(NavInputThrottleTimerHandle);
}

bool URHInputManager::IsNavInputThrottled(ERHLastInputType InputType)
{
	return GetWorld()->GetTimerManager().IsTimerActive(NavInputThrottleTimerHandle) || IsNavInputDebounceThrottled(InputType);
}

bool URHInputManager::GetButtonForInputAction(UInputAction* Action, FKey& Button, bool IsGamepadKey /*= false*/)
{
	if (Action != nullptr)
	{
		// First check the Context Mappings, which have a higher priority
		if (ContextualMappingContext != nullptr)
		{
			TArray<FKey> BoundKeys = GetBoundKeysForInputActionInMappingContext(Action, ContextualMappingContext);
			for (const FKey& Key : BoundKeys)
			{
				if (Key.IsGamepadKey() == IsGamepadKey)
				{
					Button = Key;
					return true;
				}
			}
		}

		if (MyHud != nullptr)
		{
			if (UInputMappingContext* HUDContext = MyHud->GetHUDMappingContext())
			{
				TArray<FKey> BoundKeys = GetBoundKeysForInputActionInMappingContext(Action, HUDContext);
				for (const FKey& Key : BoundKeys)
				{
					if (Key.IsGamepadKey() == IsGamepadKey)
					{
						Button = Key;
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool URHInputManager::GetAllButtonsForInputAction(UInputAction* Action, TArray<FKey>& Buttons)
{
	Buttons.Empty();

	if (Action != nullptr)
	{
		if (ContextualMappingContext != nullptr)
		{
			TArray<FKey> BoundKeys = GetBoundKeysForInputActionInMappingContext(Action, ContextualMappingContext);
			Buttons.Append(BoundKeys);
		}

		if (MyHud != nullptr)
		{
			if (UInputMappingContext* HUDContext = MyHud->GetHUDMappingContext())
			{
				TArray<FKey> BoundKeys = GetBoundKeysForInputActionInMappingContext(Action, HUDContext);
				Buttons.Append(BoundKeys);
			}
		}
	}

	return Buttons.Num() > 0;
}


TArray<FKey> URHInputManager::GetBoundKeysForInputActionInMappingContext(UInputAction* Action, UInputMappingContext* MappingContext)
{
	TArray<FKey> BoundKeys;

	if (Action != nullptr && MappingContext != nullptr)
	{
		const TArray<FEnhancedActionKeyMapping>& Mappings = MappingContext->GetMappings();
		for (const auto& Mapping : Mappings)
		{
			if (Mapping.Action == Action)
			{
				BoundKeys.AddUnique(Mapping.Key);
			}
		}
	}

	return BoundKeys;
}

UWorld* URHInputManager::GetWorld() const
{
	return GetOuter()->GetWorld();
}

/*
* Context Actions
*/

void URHInputManager::ClearContextAction(FName Route, FName ContextName)
{
	if (FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(Route))
	{
		if (ContextInfo->ClearContextAction(ContextName))
		{
			if (Route == GetCurrentContextRoute() || Route == GlobalRouteName)
			{
				UpdateActiveContexts();
			}
		}
	}
}

void URHInputManager::ClearAllContextActions(FName Route)
{
	if (FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(Route))
	{
		if (ContextInfo->ClearAllContextActions())
		{
			if (Route == GetCurrentContextRoute() || Route == GlobalRouteName)
			{
				UpdateActiveContexts();
			}
		}
	}
}

void URHInputManager::AddContextAction(FName Route, FName ContextName, FText FormatAdditive /*= FText()*/)
{
	FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(Route);
	bool bUpdated = false;

	if (!ContextInfo)
	{
		ContextInfo = &(RouteContextInfoMap.FindOrAdd(Route));
	}

	if (ContextInfo)
	{
		bUpdated = ContextInfo->AddContextAction(ContextName, ContextActionDT, FormatAdditive);
	}

	if (bUpdated && (Route == GetCurrentContextRoute() || Route == GlobalRouteName))
	{
		UpdateActiveContexts();
	}
}

void URHInputManager::AddContextActions(FName Route, TArray<FName> ContextNames)
{
	bool ContextAdded = false;

	FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(Route);

	if (!ContextInfo)
	{
		ContextInfo = &(RouteContextInfoMap.FindOrAdd(Route));
	}

	if (ContextInfo)
	{
		for (int32 i = 0; i < ContextNames.Num(); ++i)
		{
			ContextAdded = ContextInfo->AddContextAction(ContextNames[i], ContextActionDT) || ContextAdded;
		}
	}

	if (ContextAdded)
	{
		if (Route == GetCurrentContextRoute() || Route == GlobalRouteName)
		{
			UpdateActiveContexts();
		}
	}
}

void URHInputManager::UpdateActiveContexts()
{
	if (InputComponent == nullptr || ContextualMappingContext == nullptr)
	{
		return;
	}

	InputComponent->ClearActionBindings();
	ContextualMappingContext->UnmapAll();
	BindBaseHUDActions();

	// Currently Override Routes are specifically used for Popups, and we want global actions to just be available on base input.
	// TODO: If we end up having global actions become more complex or used override actions on base layers, then we need to revisit this.
	// Required Changes - Global Context Actions are registered to a layer, or all View Layers
	//					- Look up the View Layer the Top Route is in
	//					- Apply all Global Context Actions that are relevant to the routes layer

	FName TopRoute = GetCurrentRoute();

	if (FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(TopRoute))
	{
		SetInputActions((*ContextInfo).ActionData);

		if (OverrideRouteStack.Num() == 0)
		{
			FRouteContextInfo* GlobalContextInfo = RouteContextInfoMap.Find(GlobalRouteName);

			if (GlobalContextInfo)
			{
				SetInputActions((*GlobalContextInfo).ActionData);
			}
		}
	}

	OnContextActionsUpdated.ExecuteIfBound();
}

void URHInputManager::SetInputActions(TArray<UContextActionData*> ActionData)
{
	if (MyHud != nullptr && ContextualMappingContext != nullptr && InputComponent != nullptr)
	{
		// Build out action bindings based on bound actions
		for (int32 i = 0; i < ActionData.Num(); ++i)
		{
			if (ActionData[i]->RowData.Action != nullptr)
			{
				switch (ActionData[i]->RowData.ActionType)
				{
				case EContextActionType::ContextActionTypeStandard:
					MapContextualAction(ActionData[i]->RowData.Action);
					InputComponent->BindAction(ActionData[i]->RowData.Action, ETriggerEvent::Triggered, ActionData[i], &UContextActionData::TriggerContextAction);
					break;

				case EContextActionType::ContextActionTypeCycle:
					if (ActionData[i]->RowData.AltAction != nullptr)
					{
						MapContextualAction(ActionData[i]->RowData.Action);
						MapContextualAction(ActionData[i]->RowData.AltAction);
						InputComponent->BindAction(ActionData[i]->RowData.AltAction, ETriggerEvent::Triggered, ActionData[i], &UContextActionData::TriggerCycleContextActionNext);
						InputComponent->BindAction(ActionData[i]->RowData.Action, ETriggerEvent::Triggered, ActionData[i], &UContextActionData::TriggerCycleContextActionPrev);
					}
					break;

				case EContextActionType::ContextActionTypeHoldRelease:
					MapContextualAction(ActionData[i]->RowData.Action);
					InputComponent->BindAction(ActionData[i]->RowData.Action, ETriggerEvent::Triggered, ActionData[i], &UContextActionData::StartTriggerHoldAction);
					InputComponent->BindAction(ActionData[i]->RowData.Action, ETriggerEvent::Completed, ActionData[i], &UContextActionData::ClearTriggerHoldAction);
					break;

				default:
					break;
				}
			}
		}
	}
}

void URHInputManager::MapContextualAction(UInputAction* Action)
{
	ContextualMappingContext->UnmapAllKeysFromAction(Action);

	if (Action != nullptr)
	{
		TArray<FKey> MappedKeys;
		if (GetAllButtonsForInputAction(Action, MappedKeys))
		{
			for (auto Key : MappedKeys)
			{
				ContextualMappingContext->MapKey(Action, Key);
			}
		}
	}
}

void URHInputManager::SetActiveRoute(FName Route)
{
	if (Route != ActiveRoute)
	{
		ActiveRoute = Route;

		// Push the context actions of the new route to our active contexts
		if (OverrideRouteStack.Num() == 0)
		{
			UpdateActiveContexts();
		}
	}
}

bool URHInputManager::GetActiveContextActions(TArray<UContextActionData*>& TopRouteActions, TArray<UContextActionData*>& GlobalActions, FName& CurrentRoute)
{
	TopRouteActions.Empty();
	GlobalActions.Empty();

	// Currently Override Routes are specifically used for Popups, and we want global actions to just be available on base input.
	// TODO: If we end up having global actions become more complex or used override actions on base layers, then we need to revisit this.
	// Required Changes - Global Context Actions are registered to a layer, or all View Layers
	//					- Look up the View Layer the Top Route is in
	//					- Apply all Global Context Actions that are relevant to the routes layer

	bool bIsOverrideRoute = OverrideRouteStack.Num() > 0;
	CurrentRoute = bIsOverrideRoute ? OverrideRouteStack.Last() : ActiveRoute;

	if (CurrentRoute != NAME_None)
	{
		if (FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(CurrentRoute))
		{
			TopRouteActions = (*ContextInfo).ActionData;

			if (!bIsOverrideRoute)
			{
				if (FRouteContextInfo* GlobalContextInfo = RouteContextInfoMap.Find(GlobalRouteName))
				{
					GlobalActions = (*GlobalContextInfo).ActionData;
				}
			}

			return true;
		}
	}

	return false;
}

void URHInputManager::SetContextAction(FName Route, FName ContextName, const FOnContextAction& EventCallback)
{
	if (FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(Route))
	{
		ContextInfo->SetContextAction(ContextName, EventCallback);
	}
}

void URHInputManager::SetContextCycleAction(FName Route, FName ContextName, const FOnContextCycleAction& EventCallback)
{
	if (FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(Route))
	{
		ContextInfo->SetContextCycleAction(ContextName, EventCallback);
	}
}

void URHInputManager::SetContextHoldReleaseAction(FName Route, FName ContextName, const FOnContextHoldActionUpdate& UpdateCallback, const FOnContextHoldAction& EventCallback)
{
	if (FRouteContextInfo* ContextInfo = RouteContextInfoMap.Find(Route))
	{
		ContextInfo->SetContextHoldReleaseAction(ContextName, UpdateCallback, EventCallback);
	}
}

void URHInputManager::PushOverrideRoute(FName Route)
{
	// This condition can be modified if we want to allow / disallow duplicates at the top of / throughout the stack.
	// Currently blocks consecutive pushes of the same route.
	if (OverrideRouteStack.Num() == 0 || Route != OverrideRouteStack.Last())
	{
		OverrideRouteStack.Push(Route);

		// Push the contexts of the new route to the active contexts
		UpdateActiveContexts();
	}
}

FName URHInputManager::PopOverrideRoute()
{
	if (OverrideRouteStack.Num() > 0)
	{
		FName RemovedRoute = OverrideRouteStack.Pop();

		// Push the contexts of the new route to the active contexts
		UpdateActiveContexts();

		return RemovedRoute;
	}

	return NAME_None;
}

bool URHInputManager::RemoveOverrideRoute(FName RouteName)
{
	if (OverrideRouteStack.Remove(RouteName) > 0)
	{
		UpdateActiveContexts();
		return true;
	}

	return false;
}

void URHInputManager::ClearOverrideRouteStack()
{
	if (OverrideRouteStack.Num() > 0)
	{
		OverrideRouteStack.Empty();
		UpdateActiveContexts();
	}
}

FName URHInputManager::GetCurrentContextRoute() const
{
	if (OverrideRouteStack.Num() > 0)
	{
		return OverrideRouteStack.Last();
	}

	return ActiveRoute;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                                     //
//                                                                  UContextActionData                                                                 //
//                                                                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UContextActionData::Tick(float DeltaTime)
{
	// If we have an end duration, wait till we get there
	if (RowData.HoldDuration > 0)
	{
		fTickDuration += DeltaTime;

		OnHoldActionUpdate.ExecuteIfBound(fTickDuration / RowData.HoldDuration);

		if (fTickDuration >= RowData.HoldDuration)
		{
			bIsTicking = false;
			TriggerHoldReleaseContextAction(EContextActionHoldReleaseState::HoldReleaseState_Completed);
		}
	}
	else
	{
		// If we have no end duration just tick our time differential
		OnHoldActionUpdate.ExecuteIfBound(DeltaTime);
	}
}

void UContextActionData::TriggerContextAction()
{
	OnContextAction.ExecuteIfBound();
}

void UContextActionData::TriggerCycleContextAction(bool bNext)
{
	OnCycleAction.ExecuteIfBound(bNext);
}

void UContextActionData::TriggerHoldReleaseContextAction(EContextActionHoldReleaseState Status)
{
	OnHoldReleaseAction.ExecuteIfBound(Status);
}

void UContextActionData::StartTriggerHoldAction()
{
	fTickDuration = 0.f;
	bIsTicking = true;
	OnHoldActionUpdate.ExecuteIfBound(0.f);

	TriggerHoldReleaseContextAction(EContextActionHoldReleaseState::HoldReleaseState_Started);
}

void UContextActionData::ClearTriggerHoldAction()
{
	if (bIsTicking)
	{
		bIsTicking = false;
		TriggerHoldReleaseContextAction(EContextActionHoldReleaseState::HoldReleaseState_Canceled);
	}
}

TStatId UContextActionData::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UContextActionData, STATGROUP_Tickables);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                                     //
//                                                                  FRouteContextInfo                                                                  //
//                                                                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FRouteContextInfo::ClearContextAction(FName ContextName)
{
	for (int32 i = 0; i < ActionData.Num(); ++i)
	{
		if (ActionData[i]->RowName == ContextName)
		{
			ActionData[i]->ClearTriggerHoldAction();
			ActionData.RemoveAt(i);
			return true;
		}
	}

	return false;
}

bool FRouteContextInfo::ClearAllContextActions()
{
	if (ActionData.Num())
	{
		for (int32 i = 0; i < ActionData.Num(); ++i)
		{
			ActionData[i]->ClearTriggerHoldAction();
		}
		ActionData.Empty();
		return true;
	}

	return false;
}

bool FRouteContextInfo::AddContextAction(FName ContextName, UDataTable* ContextActionTable, FText FormatAdditive /*= FText()*/)
{
	FContextAction NewAction;
	if (!FindRowByName(ContextActionTable, ContextName, NewAction))
	{
		// Failed to find action
		return false;
	}

	for (int32 i = 0; i < ActionData.Num(); ++i)
	{
		// If we have no key, we just check the ContextName is not used already
		if (ActionData[i]->RowName == ContextName)
		{
			return false;
		}
	}

	// If we get to here, the given key has never been set, set it
	if (UContextActionData* NewActionData = NewObject<UContextActionData>())
	{
		NewActionData->RowName = ContextName;
		NewActionData->RowData = NewAction;
		NewActionData->FormatAdditive = FormatAdditive;
		ActionData.Add(NewActionData);
		ActionData.Sort();
		return true;
	}

	return false;
}

void FRouteContextInfo::SetContextAction(FName ContextName, const FOnContextAction& EventCallback)
{
	for (int32 i = 0; i < ActionData.Num(); ++i)
	{
		if (ActionData[i]->RowName == ContextName)
		{
			ActionData[i]->OnContextAction = EventCallback;
			return;
		}
	}
}

void FRouteContextInfo::SetContextCycleAction(FName ContextName, const FOnContextCycleAction& EventCallback)
{
	for (int32 i = 0; i < ActionData.Num(); ++i)
	{
		if (ActionData[i]->RowName == ContextName)
		{
			ActionData[i]->OnCycleAction = EventCallback;
			return;
		}
	}
}

void FRouteContextInfo::SetContextHoldReleaseAction(FName ContextName, const FOnContextHoldActionUpdate& UpdateCallback, const FOnContextHoldAction& EventCallback)
{
	for (int32 i = 0; i < ActionData.Num(); ++i)
	{
		if (ActionData[i]->RowName == ContextName)
		{
			ActionData[i]->OnHoldActionUpdate = UpdateCallback;
			ActionData[i]->OnHoldReleaseAction = EventCallback;
			return;
		}
	}
}

bool FRouteContextInfo::FindRowByName(UDataTable* ContextActionTable, FName ContextName, FContextAction& ContextActionRow)
{
	if (ContextActionTable)
	{
		FContextAction* ContextAction = ContextActionTable->FindRow<FContextAction>(ContextName, "", false);

		if (ContextAction)
		{
			ContextActionRow = *ContextAction;
			return true;
		}
	}

	return false;
}