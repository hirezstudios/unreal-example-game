// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

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