// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RHRegionSelectModal.h"

// to avoid immediately calling the region select popup again because the updated value hasn't been received yet
static bool bDidOnce = false;

bool URHRegionSelectModalViewRedirector::ShouldRedirect(ARHHUDCommon* HUD, const FGameplayTag& RouteTag, UObject*& SceneData) //$$ KAB - Route names changed to Gameplay Tags
{
	if (!bDidOnce && HUD != nullptr)
	{
		// Should redirect to this modal if the value for this user's site (region) doesn't match one of our available sites.	
		TMap<FString, FText> ValidRegionsMap;
		HUD->GetRegionList(ValidRegionsMap);
		UE_LOG(RallyHereStart, Log, TEXT("URHRegionSelectModalViewRedirector::ShouldRedirect -- Valid Region Count: %d"), ValidRegionsMap.Num());

		if (ValidRegionsMap.Num() > 0)
		{
			FString UserRegionId;
			if (HUD->GetPreferredRegionId(UserRegionId))
			{
				UE_LOG(RallyHereStart, Log, TEXT("URHRegionSelectModalViewRedirector::ShouldRedirect -- User Region Id: %s"), *UserRegionId);
				if (!ValidRegionsMap.Contains(UserRegionId))
				{
					UE_LOG(RallyHereStart, Log, TEXT("URHRegionSelectModalViewRedirector::ShouldRedirect -- User Region Id Invalid"));
					bDidOnce = true;
					return true;
				}

				UE_LOG(RallyHereStart, Log, TEXT("URHRegionSelectModalViewRedirector::ShouldRedirect -- User Region Id Valid, skipping"));
			}
			else
			{
				UE_LOG(RallyHereStart, Log, TEXT("URHRegionSelectModalViewRedirector::ShouldRedirect -- No Local Setting Found"));
				bDidOnce = true;
				return true;
			}
		}
	}

	return false;
}