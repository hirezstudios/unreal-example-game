// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHRedeemCodeScreenBase.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHRedeemCodeScreenBase : public URHWidget
{
	GENERATED_BODY()
	
public:

	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "RH|Redeem Code Screen")
	void RedeemCode(const FString& Code);

	UFUNCTION(BlueprintCallable, Category = "RH|Redeem Code Screen")
	inline bool IsPendingServerReply() const { return TimeoutDelay.IsValid(); }

protected:
	// Event called when we get a response from the redeem code or timeout
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "RH|Redeem Code Screen")
	void OnRedeemCodeResult(bool Success, const FText& Error);

	// Event called when starting a redeem request
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "RH|Redeem Code Screen")
	void OnRedeemCodeSubmit();

private:
	void OnProcessTimeout();
	void ShowGenericError();

	FString SubmittedCode;
	FTimerHandle TimeoutDelay;
};
