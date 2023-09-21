// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/Widgets/RHOverlayTabEntryWidget.h"

void URHOverlayTabEntryWidget::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	if (MyHud.IsValid())
	{
		MyHud->OnInputStateChanged.AddDynamic(this, &URHOverlayTabEntryWidget::HandleInputStateChanged);
		HandleInputStateChanged(MyHud->GetCurrentInputState());
	}

	if (HitTarget != nullptr)
	{
		HitTarget->OnHovered.AddDynamic(this, &URHOverlayTabEntryWidget::HandleHovered);
		HitTarget->OnUnhovered.AddDynamic(this, &URHOverlayTabEntryWidget::HandleUnhovered);
		HitTarget->OnClicked.AddDynamic(this, &URHOverlayTabEntryWidget::HandleClicked);
	}

	SetTabState(ERHOverlayTabState::Idle);
}

void URHOverlayTabEntryWidget::NativeGamepadHover()
{
	OnActiveViewRequested.Broadcast(MyViewName);
}

void URHOverlayTabEntryWidget::NativeGamepadUnhover()
{
	if (TabState != ERHOverlayTabState::SelectedDefocused && TabState != ERHOverlayTabState::Idle)
	{
		SetTabState(ERHOverlayTabState::Idle);
	}
}

bool URHOverlayTabEntryWidget::NavigateConfirm_Implementation()
{
	Super::NavigateConfirm_Implementation();

	OnFocusToViewRequested.Broadcast(MyViewName);

	return true;
}

/*
*	Variables
*/

void URHOverlayTabEntryWidget::SetTabState(ERHOverlayTabState InTabState)
{
	TabState = InTabState;
	DisplayTabState();
}

void URHOverlayTabEntryWidget::SetViewName(FName InViewName)
{ 
	MyViewName = InViewName;
}

void URHOverlayTabEntryWidget::SetViewInfo(FOverlayTabViewRow InViewInfo)
{ 
	MyViewInfo = InViewInfo; DisplayViewInfo();
};

/*
*	Event handlers
*/

void URHOverlayTabEntryWidget::HandleInputStateChanged_Implementation(RH_INPUT_STATE InputState)
{
}

void URHOverlayTabEntryWidget::HandleActiveViewChanged(FName ActiveView)
{
	if (ActiveView == MyViewName)
	{
		SetTabState(ERHOverlayTabState::Selected);
	}
	else
	{
		SetTabState(ERHOverlayTabState::Idle);
	}
}

void URHOverlayTabEntryWidget::HandleViewFocused()
{
	if (TabState == ERHOverlayTabState::Selected)
	{
		SetTabState(ERHOverlayTabState::SelectedDefocused);
	}
}

void URHOverlayTabEntryWidget::HandleTabsFocused()
{
	if (TabState == ERHOverlayTabState::SelectedDefocused)
	{
		SetTabState(ERHOverlayTabState::Selected);
	}
}

void URHOverlayTabEntryWidget::HandleHovered()
{
	if (TabState == ERHOverlayTabState::Idle)
	{
		SetTabState(ERHOverlayTabState::Hovered);
	}
}

void URHOverlayTabEntryWidget::HandleUnhovered()
{
	if (TabState == ERHOverlayTabState::Hovered)
	{
		SetTabState(ERHOverlayTabState::Idle);
	}
}

void URHOverlayTabEntryWidget::HandleClicked()
{
	OnActiveViewRequested.Broadcast(MyViewName);
}