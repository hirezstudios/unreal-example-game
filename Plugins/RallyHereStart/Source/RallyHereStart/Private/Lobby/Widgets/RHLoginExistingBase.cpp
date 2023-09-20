// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "DataFactories/RHLoginDataFactory.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Managers/RHViewManager.h"
#include "Lobby/Widgets/RHLoginExistingBase.h"

bool URHRedirectToLoginViewRedirector::ShouldRedirect(ARHHUDCommon* HUD, FName Route, UObject*& SceneData)
{
	if (URHLoginDataFactory* LoginDataFactory = HUD->GetLoginDataFactory())
	{
		if (LoginDataFactory->GetCurrentLoginState() == ERHLoginState::ELS_LoggedOut)
		{
			if (URHViewManager* ViewManager = HUD->GetViewManager())
			{
				FViewRoute ViewRoute;
				if (ViewManager->GetViewRoute(Route, ViewRoute))
				{
					if (ViewRoute.RequiresLoggedIn)
					{
						return true;
					}
				}
			}
		}

	}

	return false;
}