//$$ JJJT: New File
#include "Shared/Widgets/RHActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"
#include "Logging/StructuredLog.h"
#include "CommonButtonBase.h"
#include "RallyHereStart.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(RHActivatableWidget)


URHActivatableWidget::URHActivatableWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bSupportsActivationFocus = false; // default to false as we're using this as the base class for RHwidgets to maintain the RH inheritance tree
}

#pragma region Lyra

TOptional<FUIInputConfig> URHActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case ERHWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case ERHWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case ERHWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case ERHWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR
void URHActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(URHActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == URHActivatableWidget::StaticClass())
		{
			CompileLog.Warning(FText::FromString(TEXT("GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.")));
		}
		else
		{
			//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(FText::FromString("GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}
#endif

#pragma endregion Lyra

#pragma region Hemingway
void URHActivatableWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	ForEveryInteractableWidget(true);
	ForEveryEnablingWidget(true);

}

void URHActivatableWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	ForEveryInteractableWidget(false);
	ForEveryEnablingWidget(false);
}


void URHActivatableWidget::ForEveryInteractableWidget(bool bIsInFocus) const
{
	TArray<UWidget*> Interactables = GetInteractableWidgets();
	for (UWidget* wid : Interactables)
	{
		if (!wid)
		{
			continue;
		}
		if (UCommonButtonBase* AsButton = Cast<UCommonButtonBase>(wid))
		{
			AsButton->SetIsInteractionEnabled(bIsInFocus);
		}
		else
		{
			UE_LOGFMT(RallyHereStart, Warning, "Widget {0} returned by {1}::GetInteractableWidgets is of a type we don't know how to handle, and will need to be added to C++.", wid->GetName(), GetName());
		}
	}
}

void URHActivatableWidget::ForEveryEnablingWidget(bool bIsInFocus) const
{
	TArray<UWidget*> Enables = GetEnableWidgets();
	for (UWidget* Wid : Enables)
	{
		if (Wid)
		{
			Wid->SetIsEnabled(bIsInFocus);
		}
	}
}

#pragma endregion Hemingway