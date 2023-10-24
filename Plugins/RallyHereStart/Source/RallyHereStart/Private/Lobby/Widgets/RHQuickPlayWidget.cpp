// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/Widgets/RHQuickPlayWidget.h"
#include "GameFramework/RHGameInstance.h"
#include "DataFactories/RHQueueDataFactory.h"

namespace
{
	float QuickPlay_GetTimeUntilNextHalfSecond(const UWorld& World)
	{
		float Offset = 1.f - FMath::Frac(World.GetUnpausedTimeSeconds());
		if (Offset > 0.5f)
		{
			return Offset - 0.5f;
		}
		return Offset;
	}
}

URHQuickPlayWidget::URHQuickPlayWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
	CurrentQuickPlayState(EQuickPlayQueueState::Unknown),
	bPendingQueueUpdate(false),
	ElapsedTimer(), PenaltyTimer()
{
}

void URHQuickPlayWidget::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	SetupBindings();
	UpdateState();
}

//$$ LDP - Added Visibility param
void URHQuickPlayWidget::ShowWidget(ESlateVisibility InVisibility /* = ESlateVisibility::SelfHitTestInvisible */)
{
	Super::ShowWidget(InVisibility);

	UpdateState();
}

void URHQuickPlayWidget::OnQueuePermissionUpdate_Implementation(bool CanQueue)
{
	Super::OnQueuePermissionUpdate_Implementation(CanQueue);

	UpdateState();
}

void URHQuickPlayWidget::OnControlQueuePermissionUpdate_Implementation(bool CanControl)
{
	Super::OnControlQueuePermissionUpdate_Implementation(CanControl);

	UpdateState();
}

void URHQuickPlayWidget::OnSelectedQueueUpdate_Implementation(URH_MatchmakingQueueInfo* CurrentSelectedQueue)
{
	Super::OnSelectedQueueUpdate_Implementation(CurrentSelectedQueue);

	UpdateState();
}

void URHQuickPlayWidget::OnQueueStateUpdate_Implementation(ERH_MatchStatus CurrentMatchStatus)
{
	Super::OnQueueStateUpdate_Implementation(CurrentMatchStatus);

	if (CurrentMatchStatus == ERH_MatchStatus::Queued)
	{
		if (!ElapsedTimer.IsValid())
		{
			if (auto* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					ElapsedTimer,
					this, &URHQuickPlayWidget::HandleElapsedTimer,
					0.5f, true,
					QuickPlay_GetTimeUntilNextHalfSecond(*World)
				);
			}
		}
	}
	else if (ElapsedTimer.IsValid())
	{
		ElapsedTimer.Invalidate();
		OnUpdateQueueTimeElapsed(0.f);
	}

	UpdateState();
}

void URHQuickPlayWidget::UpdateState()
{
	EQuickPlayQueueState NewQuickPlayState = DetermineQuickPlayState();
	// If we get a reply back from the server that keeps us in the same state as unqueued, we need to still process this, so this resets properly
	if (bPendingQueueUpdate || CurrentQuickPlayState != NewQuickPlayState)
	{
		CurrentQuickPlayState = NewQuickPlayState;
		switch (CurrentQuickPlayState)
		{
		case EQuickPlayQueueState::ReadyToJoin:
			OnUpdateQuickPlayCanPlay(true);
			// reset cached time elapsed in queue
			OnUpdateQueueTimeElapsed(0.f);
			break;
		case EQuickPlayQueueState::ReadyToRejoin:
			OnUpdateQuickPlayCanPlay(true);
			break;
		case EQuickPlayQueueState::NoQueuesAvailable:
		case EQuickPlayQueueState::NoSelectedQueue:
		case EQuickPlayQueueState::SelectedQueueUnavailable:
		case EQuickPlayQueueState::SelectedQueuePartyMaxLimit:
		case EQuickPlayQueueState::WaitingForLeader:
			// reset cached time elapsed in queue
			OnUpdateQueueTimeElapsed(0.f);
			// fall through
		case EQuickPlayQueueState::Queued:
		case EQuickPlayQueueState::EnteringMatch:
			// fall through
		default:
			OnUpdateQuickPlayCanPlay(false);
			break;
		}
		OnUpdateQuickPlayState(CurrentQuickPlayState);
	}
}

void URHQuickPlayWidget::UpdateQueueSelection()
{
	URHQueueWidgetBase::UpdateQueueSelection();

	// Force an update so that we can update the queue name etc.
	OnUpdateQuickPlayState(CurrentQuickPlayState);
}

EQuickPlayQueueState URHQuickPlayWidget::DetermineQuickPlayState() const
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		if (QueueDataFactory->IsInitialized() && QueueDataFactory->GetQueues().Num() > 0)
		{
			// check for queue state
			if (QueueDataFactory->GetCurrentQueueMatchState() >= ERH_MatchStatus::Queued)
			{
				if (QueueDataFactory->GetCurrentQueueMatchState() >= ERH_MatchStatus::Invited)
				{
					// check if in a match and eligible for rejoin
					if (QueueDataFactory->GetCurrentQueueMatchState() > ERH_MatchStatus::Waiting)
					{
						return EQuickPlayQueueState::ReadyToRejoin;
					}
					return EQuickPlayQueueState::EnteringMatch;
				}
				return EQuickPlayQueueState::Queued;
			}

			// when you're not the leader, you must wait
			URHPartyManager* PartyManager = GetPartyManager();
			if (PartyManager != nullptr)
			{
				if (PartyManager->IsInParty() && !PartyManager->IsLeader())
				{
					return EQuickPlayQueueState::WaitingForLeader;
				}
			}

			// check for no selected queue
			if (GetCurrentlySelectedQueue() == nullptr)
			{
				return EQuickPlayQueueState::NoSelectedQueue;
			}

			return GetSelectedQueueState();
		}
	}
	// check if we can queue 
	return EQuickPlayQueueState::NoQueuesAvailable;
}

EQuickPlayQueueState URHQuickPlayWidget::GetSelectedQueueState() const
{
	URH_MatchmakingQueueInfo* CurrentQueue = GetCurrentlySelectedQueue();

	// TODO: Add in checking if we can currently join a queue
	/*if (auto* QueueData = GetQueueDataFactory())
	{
		ERHQueueJoinError Error = QueueData->CheckQueueJoinable(CurrentQueue);
		switch (Error)
		{
		case ERHQueueJoinError::None:
			break;
		case ERHQueueJoinError::SystemError:
			return EQuickPlayQueueState::Unknown;
		case ERHQueueJoinError::QueueUnavailable:
			return EQuickPlayQueueState::SelectedQueueUnavailable;
		case ERHQueueJoinError::DeserterPenaltyActive:
			return EQuickPlayQueueState::DeserterPenaltyActive;
		case ERHQueueJoinError::PartyMinMemberRequirement:
			return EQuickPlayQueueState::SelectedQueuePartyMinLimit;
		case ERHQueueJoinError::PartyMaxMemberRequirement:
			return EQuickPlayQueueState::SelectedQueuePartyMaxLimit;
		case ERHQueueJoinError::PlayerLevelRequirement:
			return EQuickPlayQueueState::PlayerLevelRequirement;
		case ERHQueueJoinError::PartyMemberLevelRequirement:
			return EQuickPlayQueueState::PartyLevelRequirement;
		case ERHQueueJoinError::PartyMemberRankRequirement:
			return EQuickPlayQueueState::PartyRankRequirement;
		case ERHQueueJoinError::PartyMemberPlatformRequirement:
			return EQuickPlayQueueState::PartyPlatformRequirement;
		case ERHQueueJoinError::PlayerOwnedJobRequirement:
			return EQuickPlayQueueState::PlayerOwnedJobRequirement;
		case ERHQueueJoinError::PartyMemberOwnedJobRequirement:
			return EQuickPlayQueueState::PartyOwnedJobRequirement;
		case ERHQueueJoinError::InQueue:
			return EQuickPlayQueueState::Queued;
		default:
			UE_LOG(RallyHereStart, Error, TEXT("RHQuickPlayWidget::GetSelectedQueueState Unhandled queue error"));
			return EQuickPlayQueueState::Unknown;
		}
	}*/

	// no special case, leggo
	return EQuickPlayQueueState::ReadyToJoin;
}

bool URHQuickPlayWidget::GetGameModeDisplayName(FText& GameModeDisplayName)
{
	if (URH_MatchmakingQueueInfo* Queue = GetCurrentlySelectedQueue())
	{
		if (auto* QueueDataFactory = GetQueueDataFactory())
		{
			FRHQueueDetails QueueDetails;
			if (QueueDataFactory->GetQueueDetailsByQueue(Queue, QueueDetails))
			{
				GameModeDisplayName = QueueDetails.Name;
				return true;
			}
			else
			{
				GameModeDisplayName = FText::FromString(FString::Printf(TEXT("NO QUEUE DETAILS: %s"), *Queue->GetQueueId()));
				return true;
			}
		}
	}
	return false;
}

void URHQuickPlayWidget::HandleElapsedTimer()
{
	if (auto* QueueData = GetQueueDataFactory())
	{
		if (QueueData->IsInQueue())
		{
			OnUpdateQueueTimeElapsed(QueueData->GetTimeInQueueSeconds());
		}
		else
		{
			ElapsedTimer.Invalidate();
			OnUpdateQueueTimeElapsed(0.f);
		}
	}
	else
	{
		ElapsedTimer.Invalidate();
		OnUpdateQueueTimeElapsed(0.f);
	}
}