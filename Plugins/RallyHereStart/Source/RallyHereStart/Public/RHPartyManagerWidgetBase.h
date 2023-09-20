// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHPartyManager.h"
#include "RHPartyManagerWidgetBase.generated.h"

/**
 *
 */
UCLASS()
class RALLYHERESTART_API URHPartyManagerWidgetBase : public URHWidget
{
	GENERATED_BODY()

public:
	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;


protected:
	UFUNCTION(BlueprintPure)
	URHPartyManager* GetPartyManager();

	UFUNCTION(BlueprintCallable)
	void RefreshFromPartyData();

	UFUNCTION(BlueprintCallable)
	void ApplyEmptyPartyData();

	UFUNCTION()
	void HandlePartyMemberDataUpdated(FRH_PartyMemberData MemberData);

	UFUNCTION()
	void HandlePartyMemberUpdateById(const FGuid& PlayerId);

	UFUNCTION()
	void HandlePartyMemberUpdateByName(FText PlayerName);

	UFUNCTION()
	void HandlePartyMemberUpdateByInfo(URH_PlayerInfo* PlayerInfo);

	UFUNCTION(BlueprintImplementableEvent)
	void ApplyPartyData(const TArray<FRH_PartyMemberData>& PartyMembers);

	UFUNCTION(BlueprintPure)
	URH_PlayerInfo* GetSuggestedInvite();

	UFUNCTION(BlueprintPure)
	TArray<FRH_PartyMemberData> GetCachedDisplayedPartyMembers() { return CachedDisplayedPartyMembers; };

	UPROPERTY()
	TArray<FRH_PartyMemberData> CachedDisplayedPartyMembers;
};
