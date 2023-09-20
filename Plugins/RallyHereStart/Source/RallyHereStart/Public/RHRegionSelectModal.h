// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Managers/RHViewManager.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHRegionSelectModal.generated.h"

UCLASS()
class RALLYHERESTART_API URHRegionSelectModalViewRedirector : public URHViewRedirecter
{
	GENERATED_BODY()

public:
	virtual bool ShouldRedirect(ARHHUDCommon* HUD, FName Route, UObject*& SceneData) override;
};

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHRegionSelectModal : public URHWidget
{
	GENERATED_BODY()
	
	// Add native functionality here
};
