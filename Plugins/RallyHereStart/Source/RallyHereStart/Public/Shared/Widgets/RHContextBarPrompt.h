// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once
#include "Managers/RHInputManager.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHContextBarPrompt.generated.h"

/**
 * Context Bar Prompt Widget
 */
UCLASS()
class RALLYHERESTART_API URHContextBarPrompt : public URHWidget
{
	GENERATED_UCLASS_BODY()

public:
	virtual void InitializeWidget_Implementation() override;

	UFUNCTION()
	void HandleInputStateChanged(RH_INPUT_STATE InputState);

	// Returns if the given container has context actions for the given input state
	bool IsValidForInput(RH_INPUT_STATE InputState);

	// Sets the ContextActionData and updates the prompts display with it
	void SetContextActionData(UContextActionData* InData);

	UFUNCTION(BlueprintImplementableEvent)
	void OnContextActionUpdated(UContextActionData* InData);

	UFUNCTION(BlueprintImplementableEvent)
	void OnInputStateChanged(RH_INPUT_STATE InputState);

	void SetPoolability(bool Poolability) { IsPoolable = Poolability; }
	bool CanBePooled() const { return IsPoolable; }

	void SetActive(bool Active) { IsActive = Active; }

	UFUNCTION(BlueprintPure, Category="Context Action")
	UContextActionData* GetContextActionData() const { return ContextActionData; }

protected:

	// All of the data associated with this given context prompt
	UPROPERTY(BlueprintReadOnly, Category="Context Action")
	UContextActionData* ContextActionData;

	// Tracks if the widget is currently poolable
	UPROPERTY(BlueprintReadWrite, Category="Pool")
	bool IsPoolable;

private:
	// This tracks if the Prompt is actually on the bar at the given time
	bool IsActive;
};