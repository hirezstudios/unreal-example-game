// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Widgets/RHMatchIdWidget.h"

FText URHMatchIdWidget::GetShortMatchId() const
{
	/* #RHTODO - PLAT-4596 
	if (auto MctsClient = pcomGetClient(0))
	{
		if (CMctsMatchMaking* MatchMaking = MctsClient->GetMatchMaking())
		{
			if (CMctsMatchGame* MatchGame = MatchMaking->GetMatch())
			{
				FString MatchId = *to_fstring(MatchGame->GetMatchId());
				return FText::FromString(MatchId.Left(8));
			}
		}
	}
	*/
	return FText::FromString(TEXT(""));
}

FText URHMatchIdWidget::GetLongMatchId() const
{
	/* #RHTODO - PLAT-4596 
	if (auto MctsClient = pcomGetClient(0))
	{
		if (CMctsMatchMaking* MatchMaking = MctsClient->GetMatchMaking())
		{
			if (CMctsMatchGame* MatchGame = MatchMaking->GetMatch())
			{
				FString MatchId = *to_fstring(MatchGame->GetMatchId());
				return FText::FromString(MatchId);
			}
		}
	}
	*/
	return FText::FromString(TEXT(""));
}
