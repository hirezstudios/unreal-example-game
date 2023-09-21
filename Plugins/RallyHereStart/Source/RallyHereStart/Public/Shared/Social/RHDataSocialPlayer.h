// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RHDataSocialPlayer.generated.h"

/**
 * Wrapper class for RHPlayerInfo data for the social menu.
 *	Circumvents TreeView's limit of no duplicates of data items
 *	by using a unique wrapper for the same player in different categories
 */
UCLASS(BlueprintType)
class RALLYHERESTART_API URHDataSocialPlayer : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRHSocialRHPlayerUpdate, URH_RHFriendAndPlatformFriend*, Friend);

	URHDataSocialPlayer();

	UFUNCTION(BlueprintPure, Category = "RH|DataSocialPlayer")
	inline URH_RHFriendAndPlatformFriend* GetFriend() const { return Friend; }

	void SetFriend(URH_RHFriendAndPlatformFriend* Player);

	UFUNCTION(BlueprintPure, Category="RH|DataSocialPlayer")
	inline bool IsValid() const { return Friend != nullptr; }

	inline operator bool() const { return IsValid(); }

	inline URH_RHFriendAndPlatformFriend* operator->() const { return Friend; }

	inline URH_RHFriendAndPlatformFriend& operator*() const { return *Friend; }

	inline operator URH_RHFriendAndPlatformFriend*() const { return Friend; }

	UPROPERTY(BlueprintAssignable, Category = "RH|Social Player")
	FRHSocialRHPlayerUpdate OnRhDataUpdate;

private:
	UPROPERTY()
	URH_RHFriendAndPlatformFriend* Friend;

	friend class URHDataSocialCategory;
};
