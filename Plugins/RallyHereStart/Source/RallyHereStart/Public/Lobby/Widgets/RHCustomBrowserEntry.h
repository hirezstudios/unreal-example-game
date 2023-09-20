// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHCustomBrowserEntry.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHCustomBrowserEntry : public URHWidget
{
	GENERATED_BODY()
	
	URHCustomBrowserEntry(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Custom Browser Entry")
	void SetCustomGameInfo(class URH_SessionView* InSession);

	UFUNCTION(BlueprintCallable, Category = "Custom Browser Entry")
	bool JoinGame();

protected:
	class URH_SessionView* CorrespondingSession;

	UFUNCTION(BlueprintPure, Category = "Custom Browser Entry|Helper")
	FString GetGameDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Custom Browser Entry|Helper")
	int32 GetPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "Custom Browser Entry | Helper")
	int32 GetMaxPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "Custom Browser Entry|Helper")
	class URHQueueDataFactory* GetQueueDataFactory() const;
};
