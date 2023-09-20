// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Lobby/Widgets/RHQueueWidgetBase.h"
#include "RHQueueTimerWidgetBase.generated.h"

UENUM(BlueprintType)
enum class EQueueTimerState : uint8
{
	Unknown                                 UMETA(DisplayName = "Unknown"),
	WaitingForLeader                        UMETA(DisplayName = "Waiting For Party Leader"),
	Queued                                  UMETA(DisplayName = "Queued"),
	EnteringMatch                           UMETA(DisplayName = "Entering Match"),
};

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHQueueTimerWidgetBase : public URHQueueWidgetBase
{
	GENERATED_BODY()

	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

	virtual void ShowWidget() override;

protected:
	virtual void OnQueuePermissionUpdate_Implementation(bool CanQueue) override;
	virtual void OnControlQueuePermissionUpdate_Implementation(bool CanControl) override;
	virtual void OnSelectedQueueUpdate_Implementation(URH_MatchmakingQueueInfo* CurrentSelectedQueue) override;
	virtual void OnQueueStateUpdate_Implementation(ERH_MatchStatus CurrentMatchStatus) override;

	void UpdateElapsedTimer(); // Handles manual incrementing
	void UpdateTimerState();

	void StartElapsedTimerHandle();
	
	void ClearTimerHandle();
	
public:
	URHQueueTimerWidgetBase();

	// Dispatched when the queue state changes in a way a timer display cares about
	UFUNCTION(BlueprintImplementableEvent, Category="RH|Queue Timer Widget")
	void OnUpdateQueueTimerState(EQueueTimerState State);

	// Dispatched when the queue time changes. Guaranteed to be after the timer state + state event
	UFUNCTION(BlueprintImplementableEvent, Category="RH|Queue Timer Widget")
	void OnUpdateQueueTime(float TimeSecs);

	UFUNCTION(BlueprintPure, Category="RH|Queue Timer Widget")
	inline EQueueTimerState GetCurrentTimerState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category="RH|Queue Timer Widget")
	inline float GetQueueTime_TotalSecs() const { return QueueTime; }

	UFUNCTION(BlueprintPure, Category="RH|Queue Timer Widget")
	inline int32 GetQueueTime_PartSecs() const { return static_cast<int32>(QueueTime) % 60; }

	UFUNCTION(BlueprintPure, Category="RH|Queue Timer Widget")
	inline int32 GetQueueTime_PartMins() const { return (static_cast<int32>(QueueTime) / 60) % 60; }

	UFUNCTION(BlueprintPure, Category="RH|Queue Timer Widget")
	inline int32 GetQueueTime_PartHours() const { return (static_cast<int32>(QueueTime) / 3600); }

protected:
	FTimerHandle UpdateTimerHandle;
	EQueueTimerState CurrentState;
	float QueueTime;
};
