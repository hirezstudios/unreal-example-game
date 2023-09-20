// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/Widgets/RHViewRedirector_LocalSetting.h"
#include "GameFramework/RHGameUserSettings.h"

bool URHViewRedirector_LocalSetting::ShouldRedirect(ARHHUDCommon* HUD, FName Route, UObject*& SceneData)
{
	if (URHGameUserSettings* const pGameSettings = Cast<URHGameUserSettings>(GEngine->GetGameUserSettings()))
	{
		if (!pGameSettings->IsLocalActionSaved(LocalActionName))
		{
			return DoesLocalSettingApply(HUD);
		}
	}

	return false;
}

bool URHViewRedirector_LocalSetting::DoesLocalSettingApply_Implementation(ARHHUDCommon* HUD) const
{
	return true;
}