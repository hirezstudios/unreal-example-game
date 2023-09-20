// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Social/RHDataSocialPlayer.h"

URHDataSocialPlayer::URHDataSocialPlayer() : UObject(), Friend(nullptr)
{ }

void URHDataSocialPlayer::SetFriend(URH_RHFriendAndPlatformFriend* Player)
{
	if (Friend != Player)
	{
		Friend = Player;
		OnRhDataUpdate.Broadcast(Friend);
	}
}
