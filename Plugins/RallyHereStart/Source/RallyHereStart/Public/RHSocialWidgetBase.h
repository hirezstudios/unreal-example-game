// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RH_FriendSubsystem.h"
#include "Managers/RHPartyManager.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RHSocialWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSocialWidgetBase : public URHWidget
{
	GENERATED_BODY()
public:
	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

	UFUNCTION(BlueprintImplementableEvent)
	void HandleFriendDataUpdated();

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void HandlePartyDataUpdated();

	UFUNCTION(BlueprintPure)
	int32 GetOnlineFriendCount() const;

	UFUNCTION(BlueprintPure)
	int32 GetIncomingInvitesCount() const;

	UFUNCTION()
	void HandleSpecificPartyDataAdded(FRH_PartyMemberData PartyMember);

	UFUNCTION()
	void HandleSpecificPartyDataUpdated(FRH_PartyMemberData PartyMember);

	UFUNCTION()
	void HandleSpecificPartyIdDataUpdated(const FGuid& PlayerId);

	UFUNCTION()
	void RHUpdateFriends(const TArray<URH_RHFriendAndPlatformFriend*>& UpdatedFriends);

	void RHUpdateFriend(URH_RHFriendAndPlatformFriend* Friend);

	UFUNCTION(BlueprintPure)
	URHPartyManager* GetPartyManager();

	URH_FriendSubsystem* GetRH_LocalPlayerFriendSubsystem() const;
	URH_PlayerInfoSubsystem* GetRH_PlayerInfoSubsystem() const;
};
