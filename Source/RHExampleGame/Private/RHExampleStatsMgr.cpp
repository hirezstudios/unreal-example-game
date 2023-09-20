#include "RHExampleGame.h"
#include "RHExampleStatsMgr.h"

float URHExampleStatsMgr::GetGameTimeElapsed(FRHStatsTracker* tracker)
{
	if (m_pGameMode != nullptr)
	{
		float GameModeTimeElapsed = m_pGameMode->GetMatchTimeElapsed();
		if (GameModeTimeElapsed > 0.0f)
		{
			return GameModeTimeElapsed;
		}
	}

	return Super::GetGameTimeElapsed(tracker);
}
