// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHJsonDataFactory.h"
#include "Managers/RHViewManager.h"
#include "Engine/Texture2DDynamic.h"
#include "RHWhatsNewModal.generated.h"

namespace RHPanel
{
	bool PanelSort(const TSharedPtr<FJsonValue> A, const TSharedPtr<FJsonValue> B);
}

// sub-panels with their own title and description
USTRUCT(BlueprintType)
struct RALLYHERESTART_API FSubPanel
{
	GENERATED_USTRUCT_BODY()

	FSubPanel()
	: Header(FText()), Desc(FText())
	{}

	FSubPanel(FText InHeader, FText InDesc)
	: Header(InHeader), Desc(InDesc)
	{}

	UPROPERTY(BlueprintReadOnly)
	FText Header;

	UPROPERTY(BlueprintReadOnly)
	FText Desc;
};

UENUM(BlueprintType)
enum class ESubPanelAlignment : uint8
{
	ESubPanelAlignment_Horizontal             UMETA(DisplayName = "Horizontal"),
	ESubPanelAlignment_VerticalLeft           UMETA(DisplayName = "Vertical Left"),
	ESubPanelAlignment_VerticalRight          UMETA(DisplayName = "Vertical Right"),
};

UENUM(BlueprintType)
enum class ENewsHeaderAlignment : uint8
{
	ENewsHeaderAlignment_Center				  UMETA(DisplayName = "Center-aligned"),
	ENewsHeaderAlignment_Left				  UMETA(DisplayName = "Left-aligned"),
};

UCLASS(BlueprintType)
class RALLYHERESTART_API URHWhatsNewPanel : public URHJsonData
{
    GENERATED_BODY()

public:
    // Header text for the panel
    UPROPERTY(BlueprintReadOnly)
    FText Header;

	// Sub-header text for panel
	UPROPERTY(BlueprintReadOnly)
	FText SubHeader;

	// alignment option for the header block
	UPROPERTY(BlueprintReadOnly)
	ENewsHeaderAlignment HeaderAlignment;

	// array of sub-panels inside the popup
	UPROPERTY(BlueprintReadOnly)
	TArray<FSubPanel> SubPanels;

	// alignment option for the body text blocks
	UPROPERTY(BlueprintReadOnly)
	ESubPanelAlignment Alignment;

	// Background box opacity
	UPROPERTY(BlueprintReadOnly)
	int32 BgBoxOpacity;

    // Image reference if available
    UPROPERTY(BlueprintReadOnly)
    UTexture2DDynamic* Image;

    // URL that is linked to by the panel directly
    UPROPERTY(BlueprintReadOnly)
    FString URL;
};

UDELEGATE()
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetWhatsNewPanelDataDynamicDelegate, const TArray<URHWhatsNewPanel*>&, PanelData);
DECLARE_DELEGATE_OneParam(FOnGetWhatsNewPanelDataDelegate, const TArray<URHWhatsNewPanel*>&);
DECLARE_RH_DELEGATE_BLOCK(FOnGetWhatsNewPanelDataBlock, FOnGetWhatsNewPanelDataDelegate, FOnGetWhatsNewPanelDataDynamicDelegate, const TArray<URHWhatsNewPanel*>&);

/**
 * Whats New modal widget
 */
UCLASS()
class RALLYHERESTART_API URHWhatsNewModal : public URHWidget
{
    GENERATED_BODY()

public:
	URHWhatsNewModal(const FObjectInitializer& ObjectInitializer);

    virtual void InitializeWidget_Implementation() override;
    virtual void UninitializeWidget_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "Whats New Modal Widget", meta = (DisplayName = "Get Panel Data Async", AutoCreateRefTerm = "Delegate"))
	void BLUEPRINT_GetPanelDataAsync(const FOnGetWhatsNewPanelDataDynamicDelegate& Delegate) { GetPanelDataAsync(Delegate); }
	void GetPanelDataAsync(FOnGetWhatsNewPanelDataBlock Delegate = FOnGetWhatsNewPanelDataBlock());

	UFUNCTION(BlueprintPure, Category = "Whats New Modal Widget")
	FORCEINLINE int32 GetMaxPanelCount() const { return maxPanelCount; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Whats New Modal Widget")
	int32 maxPanelCount;

	UPROPERTY(BlueprintReadOnly, Category = "Whats New Modal Widget")
	TArray<URHWhatsNewPanel*> StoredPanels;

    UFUNCTION(BlueprintImplementableEvent)
    void OnJsonChanged();

	UFUNCTION()
	void UpdateWhatsNewPanels(const FString& JsonName);

private:
    UFUNCTION(BlueprintPure)
    URHJsonDataFactory* GetJsonDataFactory();
};