// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Inventory/RHBattlepass.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHBattlepassTrackWidget.generated.h"

UCLASS(Config=Game)
class RALLYHERESTART_API URHBattlepassTrackWidget : public URHWidget
{
    GENERATED_BODY()
		
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Rewards Track")
	int32 CurrentPage;

	UPROPERTY(BlueprintReadOnly, Category = "Rewards Track")
	int32 NumPages;

	//Gets filled in WBP_RewardsTrack OnInit
    UPROPERTY(BlueprintReadWrite, Category = "Rewards Track")
    TArray<URHWidget*> ItemButtons;

	// Sets the battlepass that is driving the reward track and kicks off data display
	UFUNCTION(BlueprintCallable, Category = "Rewards Track")
	void SetBattlepass(URHBattlepass* Battlepass);

	// Gets the reward items for a given page
	UFUNCTION(BlueprintPure, Category = "Rewards Track")
	void GetRewardItemsForPage(int32 Index, TArray<URHBattlepassRewardItem*>& RewardItems) const;

protected:
	UPROPERTY(BlueprintReadOnly)
	URHBattlepass* Battlepass;

	// Raw list of reward items to quickly scan for page results
	UPROPERTY()
	TArray<URHBattlepassRewardItem*> RewardItems;
};