// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RHLoadoutTypes.generated.h"

// All of the different types of loadouts a player may have
// NOTE: You need to update "who loadout types" in system.cfg.jinja2 if you want these to be pulled in player loadouts
UENUM(BlueprintType)
enum class ERHLoadoutTypes : uint8
{
	INVALID_LOADOUT = 0,
	PLAYER_ACCOUNT = 1,				// Stores all of the players selected loadout information
};

UENUM(BlueprintType)
enum class ERHLoadoutSlotTypes : uint8
{
	// These are the slot ids that are saved to loadouts, do not reorder
	AVATAR_SLOT = 0,
	BANNER_SLOT = 1,
	BORDER_SLOT = 2,
	TITLE_SLOT = 3,
};
