// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/Widgets/RHBattlepassRewardsTrackSegment.h"

void URHBattlepassRewardsTrackSegment::UpdateFromRewardItem(URHBattlepass* Battlepass, URHBattlepassRewardItem* BattlepassRewardItem, URH_PlayerInfo* PlayerInfo)
{
	if (Battlepass != nullptr && BattlepassRewardItem != nullptr)
	{
		Battlepass->IsOwned(PlayerInfo, FRH_GetInventoryStateDelegate::CreateWeakLambda(this, [this, Battlepass, BattlepassRewardItem, PlayerInfo](bool bHasPremiumPass)
			{
				if (BattlepassRewardItem->Track == EBattlepassTrackType::EInstantUnlock)
				{
					// If we are in instant unlock, display the bar filled based on if we own the pass.
					ApplySegmentLabel(FText::GetEmpty());
					ApplyMeterPercentages(bHasPremiumPass ? 1.f : 0.f, 0.f);
					ApplySegmentBackgroundColor(GetPremiumBackgroundColor());
				}
				else if (URHBattlepassLevel* BattlepassLevel = Battlepass->GetBattlepassLevel(BattlepassRewardItem->GetRewardLevel()))
				{
					ApplySegmentLabel(FText::AsNumber(BattlepassLevel->LevelNumber));
					Battlepass->GetTotalXpProgress(PlayerInfo, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [this, BattlepassLevel](int32 Count)
						{
							float MappedPercentInTier = FMath::GetMappedRangeValueClamped(FVector2D(BattlepassLevel->StartXp, BattlepassLevel->EndXp), FVector2D(0.f, 1.f), Count);
							ApplyMeterPercentages(MappedPercentInTier, 0.f);
							ApplySegmentBackgroundColor(GetStandardBackgroundColor());
						}));
				}

				ApplySegmentMeterColor(bHasPremiumPass ? GetBattlePassPremiumColor() : GetBattlePassFreeColor());
			}));
	}
}