// Copyright 2016-2017 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHQueuedMessageWidget.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHQueuedMessageWidget : public URHWidget
{
	GENERATED_BODY()
	
protected:
    UFUNCTION(BlueprintCallable, Category = "Widget | Messages")
    void QueueMessage(FText Message);
	
    UFUNCTION(BlueprintCallable, Category = "Widget | Messages")
    bool GetNextMessage(FText& Message);

private:
    TQueue<FText, EQueueMode::Spsc> MessageQueue;
};
