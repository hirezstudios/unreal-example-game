#pragma once

#include "CoreMinimal.h"

#include "Containers/Map.h"
#include "Player/Controllers/RHPlayerState.h"
#include "GameFramework/RHGameModeBase.h"
#include "Misc/Guid.h"
#include "RH_Properties.h"
#include "UObject/WeakObjectPtr.h"
#include "RHStatsTracker.generated.h"

class FRHStatsTracker
{
public:
	FRHStatsTracker(class URHStatsMgr* pMgr);

	void Begin(class ARHPlayerState* pPlayerState);
	void End();
	bool IsStarted() { return m_bStarted; }
	FTimespan GetTimespan();

	void SetPlayerXpEarned(float fXpPerMin);
	inline int32 GetEarnedPlayerXp() const { return m_fEarnedPlayerXp; }

	void SetBattlepassXpEarned(float fXpPerMin);
	inline int32 GetEarnedBattlepassXp() const { return m_fEarnedBattlepassXp; }

	inline const FGuid& GetPlayerUuid() const { return m_PlayerUuid; }
	inline void SetPlayerUuid(const FGuid& pPlayerUuid) { m_PlayerUuid = pPlayerUuid; }

	inline bool ShouldStopRewards() const { return m_bStopRewards; }
	inline void SetStopRewards(bool bStopRewards) { m_bStopRewards = bStopRewards; }

	inline FDateTime GetStartTime() { return m_dtStarted; }

	bool GetDropped();

private:
	URHStatsMgr* m_pStatsMgr;
	FGuid m_PlayerUuid;
	TWeakObjectPtr<class ARHPlayerState> m_pOwningPlayerState;

	FDateTime m_dtStarted;
	FDateTime m_dtEnded;

	bool m_bStarted;
	bool m_bStopRewards;
	int32 m_fEarnedPlayerXp;
	int32 m_fEarnedBattlepassXp;
};

UCLASS(Config=Game)
class RALLYHERESTART_API URHStatsMgr : public UObject
{
	GENERATED_BODY()

public:
	URHStatsMgr();
	virtual ~URHStatsMgr();

	void BeginTracker(class ARHPlayerState* pPlayerState);
	void EndTracker(class ARHPlayerState* pPlayerState);
	void SetStopTrackerRewards(class ARHPlayerState* pPlayerState, bool bStopRewards);

	inline bool AreStatsFinished() const { return m_bStatsFinished; }
	void FinishStats(class ARHGameModeBase* pGameMode);

	inline void AddTracker(const FGuid& playerId) { GetStatsTracker(playerId); };
	void Clear();
	void ClearTracker(class ARHPlayerState* pPlayerState);

	FDateTime* GetFinishedDateTime();
	virtual float GetGameTimeElapsed(FRHStatsTracker* tracker);

	bool ShouldSendPlayerRewards(FRHStatsTracker* tracker);

	UFUNCTION(BlueprintPure)
	const FRH_LootId& GetPlayerXpLootId() const { return PlayerXpLootId; }

	UFUNCTION(BlueprintPure)
	const FRH_LootId& GetBattlepassXpLootId() const { return BattlepassXpLootId; }

protected:
	TSharedPtr<FRHStatsTracker> GetStatsTracker(const FGuid&playerId);

	UPROPERTY(Config)
	FRH_LootId PlayerXpLootId;

	UPROPERTY(Config)
	FRH_LootId BattlepassXpLootId;

private:
	TMap<FGuid, TSharedPtr<FRHStatsTracker>> m_StatsTrackers;
	bool m_bStatsFinished;
	FDateTime m_dtStatsFinished;
};