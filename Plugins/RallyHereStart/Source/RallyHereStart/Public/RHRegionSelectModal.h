// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

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
	virtual bool ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData) override; //$$ KAB - Route names changed to Gameplay Tags
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
