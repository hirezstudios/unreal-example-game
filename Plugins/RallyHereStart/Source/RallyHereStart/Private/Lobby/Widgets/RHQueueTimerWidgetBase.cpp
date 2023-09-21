// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/Widgets/RHQueueTimerWidgetBase.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "DataFactories/RHQueueDataFactory.h"

const float QueueTimerUpdateDelay = 0.5f;
const float QueueTimerUpdateDelayQuick = 0.1f;

namespace // PIMPL utils
{
	EQueueTimerState DetermineQueueTimerState(ARHLobbyHUD& Hud)
	{
		const auto* QueueData = Hud.GetQueueDataFactory();

		if (QueueData != nullptr && QueueData->IsInitialized() && QueueData->GetQueues().Num() > 0)
		{
			// check for queue state
			if (QueueData->GetCurrentQueueMatchState() >= ERH_MatchStatus::Queued)
			{
				if (QueueData->GetCurrentQueueMatchState() >= ERH_MatchStatus::Invited)
				{
					return EQueueTimerState::EnteringMatch;
				}
				return EQueueTimerState::Queued;
			}

			// when you're not the leader, you must wait
			const auto* PartyData = Hud.GetPartyManager();
			if (PartyData != nullptr && PartyData->IsInParty() && !PartyData->IsLeader())
			{
				return EQueueTimerState::WaitingForLeader;
			}

		}

		return EQueueTimerState::Unknown;
	}
}

URHQueueTimerWidgetBase::URHQueueTimerWidgetBase() : URHQueueWidgetBase(),
	UpdateTimerHandle(), CurrentState(EQueueTimerState::Unknown), QueueTime(0.f)
{
}

void URHQueueTimerWidgetBase::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();
	UpdateTimerState();
}

void URHQueueTimerWidgetBase::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();
	ClearTimerHandle();
}

void URHQueueTimerWidgetBase::ShowWidget()
{
	Super::ShowWidget();
	UpdateTimerState();
}

void URHQueueTimerWidgetBase::OnQueuePermissionUpdate_Implementation(bool CanQueue)
{
	Super::OnQueuePermissionUpdate_Implementation(CanQueue);
	UpdateTimerState();
}
void URHQueueTimerWidgetBase::OnControlQueuePermissionUpdate_Implementation(bool CanControl)
{
	Super::OnControlQueuePermissionUpdate_Implementation(CanControl);
	UpdateTimerState();
}
void URHQueueTimerWidgetBase::OnSelectedQueueUpdate_Implementation(URH_MatchmakingQueueInfo* CurrentSelectedQueue)
{
	Super::OnSelectedQueueUpdate_Implementation(CurrentSelectedQueue);
	UpdateTimerState();
}
void URHQueueTimerWidgetBase::OnQueueStateUpdate_Implementation(ERH_MatchStatus CurrentMatchStatus)
{
	Super::OnQueueStateUpdate_Implementation(CurrentMatchStatus);
	UpdateTimerState();
}


void URHQueueTimerWidgetBase::UpdateElapsedTimer()
{
	if (URHQueueDataFactory* QueueData = GetQueueDataFactory())
	{
		QueueTime = QueueData->GetTimeInQueueSeconds();
	}
	else
	{
		QueueTime = 0.f;
	}
	OnUpdateQueueTime(QueueTime);
}

void URHQueueTimerWidgetBase::UpdateTimerState()
{
	EQueueTimerState NewState = EQueueTimerState::Unknown;
	if (auto* LobbyHud = Cast<ARHLobbyHUD>(MyHud))
	{
		NewState = DetermineQueueTimerState(*LobbyHud);
	}

	if (NewState != CurrentState)
	{
		switch (CurrentState)
		{
		case EQueueTimerState::Queued:
			ClearTimerHandle();
			break;
		}

		CurrentState = NewState;

		switch (CurrentState)
		{
			case EQueueTimerState::Queued:
				UpdateElapsedTimer(); // Handles dispatch as well
				StartElapsedTimerHandle();
				break;
			case EQueueTimerState::WaitingForLeader:
			case EQueueTimerState::EnteringMatch:
			case EQueueTimerState::Unknown:
			default:
				break;
		}
		OnUpdateQueueTimerState(CurrentState);
	}
}

void URHQueueTimerWidgetBase::StartElapsedTimerHandle()
{
	if (MyHud == nullptr)
	{
		return;
	}

	float TimerOffset = 0.f;
	if (auto* World = GetWorld())
	{
		// we want to start on a half second - makes it easier to be consistent
		TimerOffset = 1.f - FMath::Frac(World->GetUnpausedTimeSeconds());
		if (TimerOffset > 0.5f)
		{
			TimerOffset -= 0.5f;
		}
	}

	MyHud->GetWorldTimerManager().SetTimer(
		UpdateTimerHandle,
		this, &URHQueueTimerWidgetBase::UpdateElapsedTimer,
		QueueTimerUpdateDelay, true,
		TimerOffset
	);
}

void URHQueueTimerWidgetBase::ClearTimerHandle()
{
	if (MyHud == nullptr || !UpdateTimerHandle.IsValid())
	{
		return;
	}

	MyHud->GetWorldTimerManager().ClearTimer(UpdateTimerHandle);
	UpdateTimerHandle.Invalidate();
}