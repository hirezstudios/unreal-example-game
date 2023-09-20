#pragma once

#include "CoreMinimal.h"
#include "Managers/RHStatsTracker.h"
#include "RHExampleGameMode.h"
#include "RHExampleStatsMgr.generated.h"

UCLASS()
class RHEXAMPLEGAME_API URHExampleStatsMgr : public URHStatsMgr
{
	GENERATED_BODY()

public:
	float GetGameTimeElapsed(FRHStatsTracker* tracker) override;

	void SetGameMode(ARHExampleGameMode* pGameMode) { m_pGameMode = pGameMode; }

private:
	ARHExampleGameMode*  m_pGameMode;
	
};
