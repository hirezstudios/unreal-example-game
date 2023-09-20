// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.

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
