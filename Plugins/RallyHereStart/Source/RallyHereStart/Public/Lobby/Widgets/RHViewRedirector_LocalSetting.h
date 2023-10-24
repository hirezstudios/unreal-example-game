// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Managers/RHViewManager.h"
#include "RHViewRedirector_LocalSetting.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class RALLYHERESTART_API URHViewRedirector_LocalSetting : public URHViewRedirecter
{
	GENERATED_BODY()
	
public:
	virtual bool ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData) override; //$$ KAB - Route names changed to Gameplay Tags

	UFUNCTION(BlueprintNativeEvent, Category = "ViewRedirector")
	bool DoesLocalSettingApply(ARHHUDCommon* HUD) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Setting")
	FName LocalActionName;
};
