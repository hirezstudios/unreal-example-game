// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHMatchIdWidget.generated.h"

UCLASS()
class RALLYHERESTART_API URHMatchIdWidget : public URHWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintPure)
	FText GetShortMatchId() const;

	UFUNCTION(BlueprintPure)
	FText GetLongMatchId() const;
};
