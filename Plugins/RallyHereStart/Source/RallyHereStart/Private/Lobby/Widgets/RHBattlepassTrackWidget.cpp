// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/Widgets/RHBattlepassTrackWidget.h"

void URHBattlepassTrackWidget::SetBattlepass(URHBattlepass* InBattlepass)
{
	Battlepass = InBattlepass;
	RewardItems.Empty();

	if (Battlepass != nullptr)
	{	
		// Append the instant unlocks first
		RewardItems.Append(Battlepass->GetInstantUnlockRewards(this));
		
		TArray<URHBattlepassLevel*> Levels = Battlepass->GetLevels(this);

		for (URHBattlepassLevel* Level : Levels)
		{
			RewardItems.Append(Level->RewardItems);
		}
	}

	if (ItemButtons.Num())
	{
		NumPages = FMath::Max(0, FMath::CeilToInt((float)RewardItems.Num() / (float)ItemButtons.Num()));
	}
	else
	{
		NumPages = 0;
	}
}

void URHBattlepassTrackWidget::GetRewardItemsForPage(int32 Index, TArray<URHBattlepassRewardItem*>& OutRewardItems) const
{	
	int32 PageSize = ItemButtons.Num();
	OutRewardItems.Empty(PageSize);

	if (Battlepass != nullptr)
	{
		int32 TargetIndex = Index * PageSize;

		// If we don't have enough items to fill the final page, back up our final index some.
		if (RewardItems.Num() - TargetIndex < PageSize)
		{
			TargetIndex = FMath::Max(0, RewardItems.Num() - PageSize);
		}

		for (int32 i = TargetIndex; i < TargetIndex + PageSize; ++i)
		{
			if (i < RewardItems.Num())
			{
				OutRewardItems.Push(RewardItems[i]);
			}
		}
	}
}