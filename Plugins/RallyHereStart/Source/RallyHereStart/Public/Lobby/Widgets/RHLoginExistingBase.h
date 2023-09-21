// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Managers/RHViewManager.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHLoginExistingBase.generated.h"

UCLASS()
class RALLYHERESTART_API URHRedirectToLoginViewRedirector : public URHViewRedirecter
{
	GENERATED_BODY()

public:
	virtual bool ShouldRedirect(ARHHUDCommon* HUD, FName Route, UObject*& SceneData);
};

/**
 * Native code for the login with existing account prompt
 */
UCLASS()
class RALLYHERESTART_API URHLoginExistingBase : public URHWidget
{
	GENERATED_BODY()
};
