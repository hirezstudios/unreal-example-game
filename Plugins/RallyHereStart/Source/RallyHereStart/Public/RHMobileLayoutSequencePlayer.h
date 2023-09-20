// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

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
