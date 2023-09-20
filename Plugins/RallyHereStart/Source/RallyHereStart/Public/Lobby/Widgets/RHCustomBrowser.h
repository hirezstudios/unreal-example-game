// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHCustomBrowser.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHCustomBrowser : public URHWidget
{
	GENERATED_BODY()

	URHCustomBrowser(const FObjectInitializer& ObjectInitializer);
	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;
	virtual void OnShown_Implementation() override;

protected:
	UFUNCTION()
	void HandleSearchResultReceived(TArray<class URH_SessionView*> CustomSessions);

	UFUNCTION(BlueprintNativeEvent, Category = "Custom Browser")
	void DisplayResult(const TArray<URH_SessionView*>& CustomSessions);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Browser")
    TSubclassOf<class URHCustomBrowserEntry> BrowserEntryWidgetClass;

	UFUNCTION(BlueprintImplementableEvent, Category = "Custom Browser")
	UPanelWidget* GetBrowserEntriesContainer() const;

	UFUNCTION(BlueprintPure, Category = "Custom Browser|Helper")
	class URHQueueDataFactory* GetQueueDataFactory() const;
};
