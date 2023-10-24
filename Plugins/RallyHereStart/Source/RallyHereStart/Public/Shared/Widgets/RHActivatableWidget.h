#pragma once
//$$ JJJT: New file


#include "CommonActivatableWidget.h"

#include "RHActivatableWidget.generated.h"

struct FUIInputConfig;

UENUM(BlueprintType)
enum class ERHWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

// An activatable widget that automatically drives the desired input config when activated
UCLASS(Abstract, Blueprintable)
class RALLYHERESTART_API URHActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	URHActivatableWidget(const FObjectInitializer& ObjectInitializer);

#pragma region Lyra Portion

public:
	
	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif
	
protected:
	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	ERHWidgetInputMode InputConfig = ERHWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input, meta=(EditCondition="InputConfig==ERHWidgetInputMode::Game||InputConfig==ERHWidgetInputMode::GameAndMenu"))
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
#pragma endregion Lyra Portion

#pragma region Hemingway Additions

public:
	void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

protected:
	void ForEveryInteractableWidget(bool bIsInFocus) const;
	void ForEveryEnablingWidget(bool bIsInFocus) const;


	/** Returns list of widgets that will be set as (un)interactable when this panel gains/loses focus **/
	UFUNCTION(BlueprintImplementableEvent, Category=Input)
	TArray<UWidget*> GetInteractableWidgets() const;

	/** Returns list of widgets that will be set as En/Disabled when this panel gains/loses focus **/
	UFUNCTION(BlueprintImplementableEvent, Category=Input)
	TArray<UWidget*> GetEnableWidgets() const;
#pragma endregion Hemingway Additions
};
