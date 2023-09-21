// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once
#include "Shared/Widgets/RHContextBarPrompt.h"
#include "Components/HorizontalBox.h"
#include "Managers/RHInputManager.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHContextBar.generated.h"

/**
 * Context Bar Widget
 */
UCLASS()
class RALLYHERESTART_API URHContextBar : public URHWidget
{
	GENERATED_UCLASS_BODY()

public:
	virtual void InitializeWidget_Implementation() override;

protected:
	// Updates the context actions displayed by the context bar
	void UpdateContextActions(const TArray<UContextActionData*>& RouteActions, const TArray<UContextActionData*>& GlobalActions, FName ActiveRoute);

	// Adds actions from a data set to the context bar
	void AddContextActions(const TArray<UContextActionData*>& RouteActions, TArray<FName>& AddList);

	// Sends an update for the current active prompts to the bar
	UFUNCTION()
	void RefreshContextBar();

	// Default Context Widget class defined on the object to be created for contexts
	UPROPERTY(EditAnywhere, Category = "Platform UMG | Context Bar")
	TSubclassOf<URHContextBarPrompt> PromptWidgetClass;

	// Margin to be applied to prompts in the LeftContainer
	UPROPERTY(EditAnywhere, Category = "Platform UMG | Context Bar")
	FMargin LeftPromptMargin;

	// Margin to be applied to prompts in the CenterContainer
	UPROPERTY(EditAnywhere, Category = "Platform UMG | Context Bar")
	FMargin CenterPromptMargin;

	// Margin to be applied to prompts in the RightContainer
	UPROPERTY(EditAnywhere, Category = "Platform UMG | Context Bar")
	FMargin RightPromptMargin;

	// Container for left aligned context prompts
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* LeftContainer;

	// Container for center aligned context prompts
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* CenterContainer;

	// Container for right aligned context prompts
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* RightContainer;

	// Pool of unused created Context Widgets to reduce deleting and recreating widgets as we change prompts
	UPROPERTY()
	TArray<URHContextBarPrompt*> PromptPool;

private:
	void ReturnContainerContentsToPool(UHorizontalBox* Container, TArray<FName>& AddList, const TArray<UContextActionData*>& RouteActions);

	// Returns if any containers currently have context actions on them
	bool HasContextActions();

	// Returns if the given container has context actions for the given input state
	bool HasContextActionForInput(UHorizontalBox* Container, RH_INPUT_STATE InputState);
};