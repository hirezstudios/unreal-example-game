// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/UMGSequencePlayer.h"
#include "RHMobileLayoutSequencePlayer.generated.h"

UCLASS()
class URHMobileLayoutSequencePlayer : public UUMGSequencePlayer
{
	GENERATED_BODY()
public:
	void ActivateMobileLayout(UWidgetAnimation& InAnimation);
	void DeactivateMobileLayout();
};
