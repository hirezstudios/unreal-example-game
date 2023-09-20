// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Social/RHSocialPanelBase.h"
#include "RHSocialFriendsPanel.generated.h"

class URHDataSocialCategory;

UENUM(BlueprintType)
enum class ERHSocialFriendSection : uint8
{
	Invalid					UMETA(DisplayName = "Invalid"),
	PartyMembers			UMETA(DisplayName = "Party Members"),
	MatchTeamMembers		UMETA(DisplayName = "Match Team Members"),
	PartyInvitations		UMETA(DisplayName = "Party Invitations"),
	OnlineRHFriends			UMETA(DisplayName = "RH Friends Online"),
	OnlinePlatformFriends	UMETA(DisplayName = "Platform Friends Online"),
	OfflineMctsFriends		UMETA(DisplayName = "Mcts Friends Offline"),
	Blocked					UMETA(DisplayName = "Blocked"),
	Pending					UMETA(DisplayName = "Pending"),
	MAX
};

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSocialFriendsPanel : public URHSocialPanelBase
{
	GENERATED_BODY()
	
public:
	URHSocialFriendsPanel();

	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

	inline const TArray<URHDataSocialCategory*>& GetData() const { return CategoryData; }

private:
	UPROPERTY(BlueprintReadOnly, Category = "RH|Social Friends", Meta = (AllowPrivateAccess = true, DisplayName = "Data"))
	TArray<URHDataSocialCategory*> CategoryData;
	UPROPERTY(Transient)
	URHSocialOverlay* Parent;
};
