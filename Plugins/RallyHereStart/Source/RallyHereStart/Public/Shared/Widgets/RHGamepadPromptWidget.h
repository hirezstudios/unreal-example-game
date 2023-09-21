// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "Shared/Widgets/RHWidget.h"
#include "RHGamepadPromptWidget.generated.h"

USTRUCT(BlueprintType)
struct FButtonPromptData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Gamepad | Prompts")
	FKey Key;

	UPROPERTY(BlueprintReadWrite, Category = "Gamepad | Prompts")
	FText Text;
};

USTRUCT(BlueprintType)
struct FButtonPromptContext
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FButtonPromptData> PromptInfo;
};

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHGamepadPromptWidget : public URHWidget
{
	GENERATED_BODY()

public:
	// Adds a set of gamepad prompts (it becomes displayed).
	UFUNCTION(BlueprintCallable, Category = "Gamepad | Prompts")
	void PushContext(FButtonPromptContext PromptContext);

	// Removes the uppermost set of gamepad prompts and displays the new uppermost context.
	// Returns false iff the context stack is empty.
	UFUNCTION(BlueprintCallable, Category = "Gamepad | Prompts")
	bool PopContext(FButtonPromptContext& OutContext);

	UFUNCTION(BlueprintCallable, Category = "Gamepad | Prompts")
	void ClearAllContext();

	// Sets the current context (pushes a new one if context is empty).
	UFUNCTION(BlueprintCallable, Category = "Gamepad | Prompts")
	void SetContext(FButtonPromptContext PromptContext);

protected:
	// Uses the given context to set up the widget display.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Gamepad | Prompts")
	void ApplyContext(FButtonPromptContext Context);

private:
	TArray<FButtonPromptContext> ContextStack;
};
