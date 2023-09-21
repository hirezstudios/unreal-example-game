// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RH_FriendSubsystem.h"
#include "RHPlayerCardModuleBase.generated.h"

UCLASS()
class RALLYHERESTART_API URHPlayerCardModuleBase : public URHWidget
{
	GENERATED_BODY()

public:
	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

	// Main function for setting the player data
	UFUNCTION(BlueprintCallable, Category = "RHPlayerCardModule")
	virtual void View_SetFriend(URH_RHFriendAndPlatformFriend* Friend);

protected:
	// Called when the player's data has changed
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "RHPlayerCardModule")
	void OnRHFriendSet(class URH_RHFriendAndPlatformFriend* Friend);

	UPROPERTY(BlueprintReadOnly, Category = "RHPlayerCardModule")
	class URH_RHFriendAndPlatformFriend* AssignedFriend;
};
