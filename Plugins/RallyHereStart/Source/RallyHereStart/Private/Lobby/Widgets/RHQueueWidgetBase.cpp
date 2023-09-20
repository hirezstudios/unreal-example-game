// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "PlatformInventoryItem/PInv_AssetManager.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Lobby/Widgets/RHQueueWidgetBase.h"
#include "Managers/RHPopupManager.h"
#include "DataFactories/RHQueueDataFactory.h"

void URHQueueWidgetBase::SetupBindings()
{
    // bind to match status update
    if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
    {
        pQueueDataFactory->OnQueueStatusChange.AddDynamic(this, &URHQueueWidgetBase::ReceiveMatchStatusUpdate);
        pQueueDataFactory->OnQueueDataUpdated.AddDynamic(this, &URHQueueWidgetBase::SetupReadyForQueueing);
		pQueueDataFactory->OnSetQueueId.AddDynamic(this, &URHQueueWidgetBase::HandleSelectedQueueIdSet);
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::SetupBindings failed to get QueueDataFactory-- delegates will not be bound."));
    }

    // bind to party status update
    if (URHPartyManager* pPartyManager = GetPartyManager())
    {
		pPartyManager->OnPartyDataUpdated.AddDynamic(this, &URHQueueWidgetBase::UpdateQueuePermissions);
		pPartyManager->OnPartyInfoUpdated.AddDynamic(this, &URHQueueWidgetBase::UpdateQueueSelection);
		pPartyManager->OnPartyMemberDataUpdated.AddDynamic(this, &URHQueueWidgetBase::HandlePartyMemberDataUpdated);
		pPartyManager->OnPartyMemberRemoved.AddDynamic(this, &URHQueueWidgetBase::HandlePartyMemberRemoved);
		pPartyManager->OnPartyMemberLeft.AddDynamic(this, &URHQueueWidgetBase::HandlePartyMemberDataUpdated);
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::SetupBindings failed to get PartyManager-- delegates will not be bound."));
    }

    // attempt to set up the queues now
    if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
    {
        if (pQueueDataFactory->IsInitialized())
        {
            SetupReadyForQueueing();
        }
    }
}

/**
* Internal helpers
**/
URHQueueDataFactory* URHQueueWidgetBase::GetQueueDataFactory() const
{
    if (MyHud.IsValid())
    {
        if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
        {
            return RHLobbyHUD->GetQueueDataFactory();
        }
        else
        {
            UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::GetQueueDataFactory Warning: MyHud failed to cast to ARHLobbyHUD."));
        }
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::GetQueueDataFactory Warning: MyHud is not currently valid."));
    }
    return nullptr;
}

URHPartyManager* URHQueueWidgetBase::GetPartyManager() const
{
    if (MyHud.IsValid())
    {
        return MyHud->GetPartyManager();
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::GetPartyManager Warning: MyHud is not currently valid."));
    }
    return nullptr;
}

/**
* State change updates
**/

void URHQueueWidgetBase::OnQueuePermissionUpdate_Implementation(bool CanQueue)
{
	return; // stub for ancestors
}

void URHQueueWidgetBase::OnControlQueuePermissionUpdate_Implementation(bool CanControl)
{
	return; // stub for ancestors
}

void URHQueueWidgetBase::OnSelectedQueueUpdate_Implementation(URH_MatchmakingQueueInfo* CurrentSelectedQueue)
{
	return; // stub for ancestors
}

void URHQueueWidgetBase::OnQueueStateUpdate_Implementation(ERH_MatchStatus CurrentMatchStatus)
{
	return; // stub for ancestors
}

void URHQueueWidgetBase::ReceiveMatchStatusUpdate(ERH_MatchStatus CurrentMatchStatus)
{
	OnQueueStateUpdate(CurrentMatchStatus);
	HandleMatchStatusUpdate(CurrentMatchStatus);
	SetupReadyForQueueing();
}

void URHQueueWidgetBase::HandlePartyMemberDataUpdated(FRH_PartyMemberData PartyMember)
{
	UpdateQueuePermissions();
}

void URHQueueWidgetBase::HandlePartyMemberRemoved(const FGuid& PartyMemberId)
{
	UpdateQueuePermissions();
}

void URHQueueWidgetBase::UpdateQueueSelection()
{
	UpdateQueuePermissions();
}

void URHQueueWidgetBase::UpdateQueuePermissions()
{
	bool CanControl = false;
	bool CanQueue = false;
	GetQueuePermissions(CanControl, CanQueue);

	OnControlQueuePermissionUpdate(CanControl);
	OnQueuePermissionUpdate(CanQueue);
}

void URHQueueWidgetBase::SetupReadyForQueueing()
{
	// trigger an update to the queue perms
	UpdateQueuePermissions();
}

void URHQueueWidgetBase::HandleSelectedQueueIdSet()
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		UE_LOG(RallyHereStart, Verbose, TEXT("URHQueueWidgetBase::HandleSelectedQueueIdSet attempting to apply Queue ID currently in Queue DataFactory as Selected."));
		SetupReadyForQueueing();
		ApplyQueueChangeHelper(pQueueDataFactory, pQueueDataFactory->GetSelectedQueueId());
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::HandleSelectedQueueIdSet failed to get queue data factory."));
	}
}

void URHQueueWidgetBase::HandleMatchStatusUpdate(ERH_MatchStatus MatchStatus)
{
	if (MatchStatus >= ERH_MatchStatus::Queued)
	{
		if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
		{
			const FString CurrentQueueId = pQueueDataFactory->GetSelectedQueueId();
			UE_LOG(RallyHereStart, Verbose, TEXT("URHQueueWidgetBase::HandleMatchStatusUpdate attempting to bring the UI's selected queue up to date: Pulled the queue ID currently in server info (%s)."), *CurrentQueueId);
			ApplyQueueChangeHelper(pQueueDataFactory, CurrentQueueId);
		}
		else
		{
			UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::HandleMatchStatusUpdate failed to get queue data factory."));
		}
	}
}

void URHQueueWidgetBase::ApplyQueueChangeHelper(URHQueueDataFactory* pQueueDataFactory, const FString& QueueId)
{
	if (pQueueDataFactory != nullptr)
	{
		URH_MatchmakingQueueInfo* SelectedQueue = GetQueueInfoById(QueueId);
		if (SelectedQueue != nullptr)
		{
			UE_LOG(RallyHereStart, Verbose, TEXT("URHQueueWidgetBase::ApplyQueueChangeHelper now applying Queue ID %s."), *QueueId);
			
			if (pQueueDataFactory->GetSelectedQueueId() != QueueId)
			{
				pQueueDataFactory->SetSelectedQueueId(QueueId);
			}

			OnSelectedQueueUpdate(SelectedQueue);
		}
		else
		{
			UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::ApplyQueueChangeHelper ignoring selected queue with invalid client info."));
		}
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::ApplyQueueChangeHelper was given nullptr for queue data factory."));
	}
}

void URHQueueWidgetBase::HandleConfirmLeaveQueue()
{
	if (auto* QueueData = GetQueueDataFactory())
	{
		QueueData->LeaveQueue();
	}
}

/**
* API
**/
TArray<URH_MatchmakingQueueInfo*> URHQueueWidgetBase::GetQueues()
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
    {
		return pQueueDataFactory->GetQueues();
    }
	
    return TArray<URH_MatchmakingQueueInfo*>();
}

URH_MatchmakingQueueInfo* URHQueueWidgetBase::GetCurrentlySelectedQueue() const
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		if (!pQueueDataFactory->GetSelectedQueueId().IsEmpty())
		{
			return pQueueDataFactory->GetQueueInfoById(pQueueDataFactory->GetSelectedQueueId());
		}
	}
	return nullptr;
}

bool URHQueueWidgetBase::SetCurrentlySelectedQueue(const FString& QueueId)
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		return pQueueDataFactory->SetSelectedQueueId(QueueId);
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::SetCurrentlySelectedQueue failed to get queue data factory."));
	}
	return false;
}

URH_MatchmakingQueueInfo* URHQueueWidgetBase::GetQueueInfoById(const FString& QueueId)
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		if (!pQueueDataFactory->GetSelectedQueueId().IsEmpty())
		{
			return pQueueDataFactory->GetQueueInfoById(QueueId);
		}
	}

	return nullptr;
}

void URHQueueWidgetBase::GetQueuePermissions(bool& CanControl, bool& CanQueue)
{
	// check if we can control the queue
	if (URHPartyManager* pPartyManager = GetPartyManager())
	{
		CanControl = (pPartyManager->IsInParty() ? pPartyManager->IsLeader() : true);
	}

	// check if we can queue
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		// we check perms but also we cannot queue if there are no queues
		CanQueue = (pQueueDataFactory->CanQueue() && pQueueDataFactory->GetQueues().Num() > 0);
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::UpdateQueuePermissions failed"));
	}
}

bool URHQueueWidgetBase::IsValidQueue(const FString& QueueId) const
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		return (pQueueDataFactory->GetQueueInfoById(QueueId) != nullptr);
	}
	return false;
}

bool URHQueueWidgetBase::UIX_AttemptJoinSelectedQueue()
{
    if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
    {
        if (pQueueDataFactory->CanQueue())
        {
            pQueueDataFactory->JoinSelectedQueue();
        }
        else
        {
            UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::UIX_AttemptJoinSelectedQueue failed; user cannot currently queue."));
        }
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::UIX_AttemptJoinSelectedQueue failed."));
    }
    return false;
}

bool URHQueueWidgetBase::UIX_AttemptCancelQueue()
{
	URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory();
    if (pQueueDataFactory == nullptr)
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::UIX_AttemptCancelQueue failed to get Queue Data Factory."));
		return false;
	}
	
	URHPartyManager* pPartyManager = GetPartyManager();
	if (pPartyManager == nullptr)
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::UIX_AttemptCancelQueue failed; user cannot currently queue."));
		return false;
	}

	if (pPartyManager->IsInParty() && !pPartyManager->IsLeader())
	{
		return false;
	}

	if (pQueueDataFactory->IsInCustomMatch() && MyHud != nullptr)
	{
		if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
		{
			if (auto* PopupManager = RHLobbyHUD->GetPopupManager())
			{
				auto PopupData = FRHPopupConfig();

				PopupData.Header = NSLOCTEXT("RHQueue", "LeaveCustomLobby", "LEAVE CUSTOM LOBBY");
				PopupData.Description = NSLOCTEXT("RHQueue", "ConfirmLeaveCustomLobby", "Are you sure you want to leave this Custom Game Lobby?");
				PopupData.IsImportant = true;
				PopupData.CancelAction.AddDynamic(PopupManager, &URHPopupManager::OnPopupCanceled);

				auto& ConfirmButton = PopupData.Buttons[PopupData.Buttons.AddDefaulted()];
				ConfirmButton.Type = ERHPopupButtonType::Confirm;
				ConfirmButton.Label = NSLOCTEXT("General", "Confirm", "Confirm");
				ConfirmButton.Action.AddDynamic(this, &URHQueueWidgetBase::HandleConfirmLeaveQueue);

				auto& promoteCancelBtn = (PopupData.Buttons[PopupData.Buttons.AddDefaulted()]);
				promoteCancelBtn.Label = NSLOCTEXT("General", "Cancel", "Cancel");
				promoteCancelBtn.Type = ERHPopupButtonType::Cancel;
				promoteCancelBtn.Action.AddDynamic(PopupManager, &URHPopupManager::OnPopupCanceled);

				PopupManager->AddPopup(PopupData);
				return true;
			}
		}
	}
	return pQueueDataFactory->LeaveQueue();
}

bool URHQueueWidgetBase::UIX_AttemptRejoinMatch()
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		return pQueueDataFactory->AttemptRejoinMatch();
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::UIX_AttemptRejoinMatch failed."));
	}
	return false;
}

bool URHQueueWidgetBase::UIX_AttemptLeaveMatch()
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		return pQueueDataFactory->LeaveMatch();
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHQueueWidgetBase::UIX_AttemptLeaveMatch failed."));
	}
	return false;
}