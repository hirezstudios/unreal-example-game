#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Shared/Widgets/RHContextBarPrompt.h"

URHContextBarPrompt::URHContextBarPrompt(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	IsPoolable = false;
}

void URHContextBarPrompt::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();
	
	if (MyHud != nullptr)
	{
		MyHud->OnInputStateChanged.AddDynamic(this, &URHContextBarPrompt::HandleInputStateChanged);
	}
}

void URHContextBarPrompt::HandleInputStateChanged(RH_INPUT_STATE InputState)
{
	// As we return these to the pool we keep the delegate active as we will commonly be pulled out and reused, but we just ignore it when not active
	if (!IsActive)
	{
		// Make sure we are collapsed if not active just in case
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	bool IsVisible = IsValidForInput(InputState);

	SetVisibility(IsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// If we are actually visible do anything we need to do for our current state in case we change display modes
	if (IsVisible)
	{
		OnInputStateChanged(InputState);
	}
}

void URHContextBarPrompt::SetContextActionData(UContextActionData* InData)
{
	ContextActionData = InData;

	// Prompts can vary from game to game so the blueprint is in charge of final display
	OnContextActionUpdated(ContextActionData);

	// Update display based on current input state
	HandleInputStateChanged(MyHud->GetCurrentInputState());
}

bool URHContextBarPrompt::IsValidForInput(RH_INPUT_STATE InputState)
{
	if (ContextActionData)
	{
		return ContextActionData->RowData.ValidInputTypes.Contains(InputState);
	}

	return false;
}