// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Engine/Texture2DDynamic.h"
#include "Subsystems/RHNewsSubsystem.h"
#include "RHNewsRotatorWidget.generated.h"

UENUM(BlueprintType)
enum class ENewsActions : uint8
{
	ENewsActions_Unknown               UMETA(DisplayName = "Unknown"),
	ENewsActions_ExternalURL           UMETA(DisplayName = "ExternalURL"),
	ENewsActions_NavToRoute            UMETA(DisplayName = "Route Navigation"),
	ENewsActions_NavToStoreItem        UMETA(DisplayName = "Store Item Navigation"),
	ENewsActions_NavToRogueDetails     UMETA(DisplayName = "Rogue Details Navigation"),
	ENewsActions_NavToCustomization	   UMETA(DisplayName = "Customization Selector"),
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHNewsRotatorData : public URHJsonData
{
    GENERATED_BODY()

public:
    // Image reference if available
    UPROPERTY(BlueprintReadOnly)
    UTexture2DDynamic* Image;

	// Header Text to display if available
	UPROPERTY(BlueprintReadOnly)
	FText Header;

	// Body Text to display if available
	UPROPERTY(BlueprintReadOnly)
	FText Body;

    // Action specific for the given panel
    UPROPERTY(BlueprintReadOnly)
    ENewsActions PanelAction;

    // Metadata used by the action (URL, Route Name, etc.)
    UPROPERTY(BlueprintReadOnly)
    FString ActionDetails;

	// Flag for click tracking
	bool bTrackClicks;

	// Number of cohort groups for click tracking/version testing
	int32 NumGroups;

	// The cohort group that this panel should be shown to (starts from 1)
	int32 GroupToShowFor;
};

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetNewsRotatorPanelsDynamicDelegate, const TArray<URHNewsRotatorData*>&, Data);
DECLARE_DELEGATE_OneParam(FOnGetNewsRotatorPanelsDelegate, const TArray<URHNewsRotatorData*>&);
DECLARE_RH_DELEGATE_BLOCK(FOnGetNewsRotatorPanelsBlock, FOnGetNewsRotatorPanelsDelegate, FOnGetNewsRotatorPanelsDynamicDelegate, const TArray<URHNewsRotatorData*>&);

/**
 * News Rotator Widget
 */
UCLASS()
class RALLYHERESTART_API URHNewsRotatorWidget : public URHWidget
{
    GENERATED_BODY()

public:
    virtual void InitializeWidget_Implementation() override;
    virtual void UninitializeWidget_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "News Rotator Widget", meta = (DisplayName = "Get Panel Data Async", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_GetPanelDataAsync(const FOnGetNewsRotatorPanelsDynamicDelegate& Delegate) { GetPanelDataAsync(Delegate); }
	void GetPanelDataAsync(FOnGetNewsRotatorPanelsBlock Delegate = FOnGetNewsRotatorPanelsBlock());

    UFUNCTION(BlueprintCallable, Category = "News Rotator Widget", meta = (DisplayName = "Check Should Show Panels", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_CheckShouldShowPanels(TArray<URHNewsRotatorData*> Panels, const FOnGetNewsRotatorPanelsDynamicDelegate& Delegate) { CheckShouldShowPanels(Panels, Delegate); }
    void CheckShouldShowPanels(TArray<URHNewsRotatorData*> Panels, FOnGetNewsRotatorPanelsBlock Delegate = FOnGetNewsRotatorPanelsBlock());

	UFUNCTION(BlueprintCallable, Category = "News Rotator Widget")
	void OnNewsPanelClicked(URHNewsRotatorData* Panel);

protected:
    UFUNCTION(BlueprintImplementableEvent)
    void OnJsonChanged(const FString& JsonName);

    // References the JSON section that is used to populate this widget
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "News Rotator Widget")
    FString JsonSection;

    // Sets the time between news section rotations
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "News Rotator Widget")
    float TimePerSection;

	//$$ KAB - Route names changed to Gameplay Tags, New Var to set ViewTag
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Views", meta = (Categories = "View"))
	FGameplayTag StoreViewTag;

private:
    UFUNCTION(BlueprintPure)
    URHNewsSubsystem* GetNewsSubsystem();
};