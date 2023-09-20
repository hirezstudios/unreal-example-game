// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RHBattlepass.h"
#include "RHProgressMeterWidgetBase.h"
#include "RHBattlepassRewardsTrackSegment.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHBattlepassRewardsTrackSegment : public URHProgressMeterWidgetBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void UpdateFromRewardItem(URHBattlepass* Battlepass, URHBattlepassRewardItem* BattlepassRewardItem, URH_PlayerInfo* PlayerInfo);

	// Functions to set the segment display elements
	UFUNCTION(BlueprintImplementableEvent)
	void ApplySegmentLabel(const FText& LabelText);

	UFUNCTION(BlueprintImplementableEvent)
	void ApplySegmentProgress(float ProgressPercent);

	UFUNCTION(BlueprintImplementableEvent)
	void ApplySegmentMeterColor(FLinearColor MeterColor);

	UFUNCTION(BlueprintImplementableEvent)
	void ApplySegmentBackgroundColor(FLinearColor BackgroundColor);

	// Functions to get design properties
	UFUNCTION(BlueprintImplementableEvent)
	FLinearColor GetBattlePassFreeColor();
	UFUNCTION(BlueprintImplementableEvent)
	FLinearColor GetBattlePassPremiumColor();
	UFUNCTION(BlueprintImplementableEvent)
	FLinearColor GetStandardBackgroundColor();
	UFUNCTION(BlueprintImplementableEvent)
	FLinearColor GetPremiumBackgroundColor();
};
