#pragma once
#include "Shared/Widgets/RHWidget.h"
#include "Engine/Texture2D.h"
#include "RHPopupManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHPopupResponse);

UENUM(BlueprintType)
enum class ERHPopupFormat : uint8
{
	Standard,
	HumanBackfillReward,
	ReportPlayerFeedback,
    GamepadPrompt
};

UENUM(BlueprintType)
enum class ERHPopupButtonType : uint8
{
    Confirm,
    Cancel,
    Default
};

USTRUCT(BlueprintType)
struct FRHPopupButtonConfig
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Popup")
    FText Label;

    UPROPERTY(BlueprintAssignable, Category = "Platform UMG | Popup")
	FRHPopupResponse Action;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Popup")
    ERHPopupButtonType Type;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Popup")
    float DelayToActivate;

    UPROPERTY(BlueprintReadOnly, Category = "Platform UMG | Popup")
    float StartDelayTimerAt;

    FRHPopupButtonConfig()
		: Label()
		, Action()
		, Type(ERHPopupButtonType::Default)
        , DelayToActivate()
        , StartDelayTimerAt()
    {
    }
};

USTRUCT(BlueprintType)
struct FRHPopupConfig
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    FText Header;

	UPROPERTY(BlueprintReadWrite, Category = "Popup")
	FText SubHeading;

	UPROPERTY(BlueprintReadWrite, Category = "Popup")
	TSoftObjectPtr<UTexture2D> HeadingIcon;

    UPROPERTY(BlueprintReadWrite, meta = (MultiLine = true), Category = "Popup")
    FText Description;

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    FText Warning;

    //$$ PGL - For AFK Warning popup
    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    float Timer;

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    bool bBlockActions;
    //$$ PGL - End

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    bool TextEntry;

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    FText TextEntryHint;

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    bool IsImportant;

    // Determines if a popup can have another popup appear above it
    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    bool CanBeShownOver;

    UPROPERTY(BlueprintReadOnly, Category = "Popup")
    bool TreatAsBlocker;

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    TArray<FRHPopupButtonConfig> Buttons;

    UPROPERTY(BlueprintAssignable, Category = "Popup")
	FRHPopupResponse CancelAction;

	UPROPERTY(BlueprintReadWrite, Category = "Popup")
	TEnumAsByte<ETextJustify::Type> TextAlignment;

    UPROPERTY(BlueprintReadOnly, Category = "Popup")
    int32 PopupId;

	UPROPERTY(BlueprintReadWrite, Category = "Popup")
	ERHPopupFormat PopupFormat;

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    FString TopImageTextureName;

    UPROPERTY(BlueprintReadWrite, Category = "Popup")
    FKey KeyToDisplay;

    FRHPopupConfig()
		: Header()
		, SubHeading()
		, HeadingIcon()
        , Description()
        , Warning()
    //$$ PGL - For AFK Warning popup
        , Timer()
        , bBlockActions(true)
    //$$ PGL - End
        , TextEntry(false)
        , TextEntryHint()
		, IsImportant(false)
        , CanBeShownOver(true)
		, TreatAsBlocker(false)
        , Buttons()
        , CancelAction()
		, TextAlignment(ETextJustify::Center)
        , PopupId(INDEX_NONE)
		, PopupFormat(ERHPopupFormat::Standard)
    {
    }

    bool IsValid()
    {
        return (PopupId != INDEX_NONE);
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowPopupEvent);
//$$ PGL - Delegate that calls when the Popup Timer finish
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerExpired);
//$$ PGL - End

UCLASS()
class RALLYHERESTART_API URHPopupManager : public URHWidget
{
    GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Popup")
    int32 AddPopup(const FRHPopupConfig &popupData);

    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Popup")
    void RemovePopup(int32 popupId);

    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Popup")
    void NextPopup();

    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Popup")
    void OnPopupCanceled();

    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Popup")
    void CloseAllPopups();

    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Popup")
    void CloseUnimportantPopups();

    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Popup")
    void OnPopupResponse(int32 nPopupId, int32 nResponseIndex);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Platform UMG | Popup")
    void ShowPopup(FRHPopupConfig popupData);

    bool HasActivePopup() { return CurrentPopup.IsValid(); }

    //CommittedText will be empty after this function is called
    FText CommittedTextHandoff();

	// Event broadcasted when a popup is shown
	FOnShowPopupEvent OnShowPopup;

    // Event broadcasted when a timer finished
    UPROPERTY(BlueprintCallable)
    FOnTimerExpired OnTimerExpired;

    // If set, whenever a popup overrides another one the old ones go into a queue to re-show
    UPROPERTY()
    bool bUsesPopupQueue;

    // If set, when a new popup is requested, it replace the currently displayed popup
    UPROPERTY()
    bool bNewPopupsShowOverCurrent;

protected:

    UFUNCTION(BlueprintImplementableEvent, Category = "Platform UMG | Popup")
    void HidePopup();

    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Popup")
    TArray<FRHPopupConfig> PopupQueue;

    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Popup")
    int32 m_nPopupId;

    FRHPopupConfig CurrentPopup;

    UPROPERTY(BlueprintReadWrite, Category = "Platform UMG | Popup")
    FText CommittedText;
};