// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RHPlayerCardModuleBase.h"

void URHPlayerCardModuleBase::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();
}

void URHPlayerCardModuleBase::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();
}

void URHPlayerCardModuleBase::View_SetFriend(URH_RHFriendAndPlatformFriend* Friend)
{
	if (Friend != nullptr)
	{
		AssignedFriend = Friend;

		OnRHFriendSet(Friend);
	}
}