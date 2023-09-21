// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "PlatformInventoryItem/PInv_AssetManager.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Lobby/Widgets/RHQuickPlay.h"
#include "DataFactories/RHQueueDataFactory.h"

URHQuickPlay::URHQuickPlay(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    DefaultSelectedQueueId = TEXT("");
    CanCurrentlyJoinQueue = false;
    CanControlQueue = false;
    ReadyForQueueing = false;
}

void URHQuickPlay::SetupBindings()
{
    // bind to match status update
    if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
    {
        pQueueDataFactory->OnQueueStatusChange.AddDynamic(this, &URHQuickPlay::ReceiveMatchStatusUpdate);
        pQueueDataFactory->OnQueueDataUpdated.AddDynamic(this, &URHQuickPlay::SetupReadyForQueueing);
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::SetupBindings failed to get QueueDataFactory-- delegates will not be bound."));
    }

    // bind to party status update
    if (URHPartyManager* pPartyManager = GetPartyManager())
    {
		pPartyManager->OnPartyDataUpdated.AddDynamic(this, &URHQuickPlay::UpdateQueuePermissions);
		pPartyManager->OnPartyMemberDataUpdated.AddDynamic(this, &URHQuickPlay::HandlePartyMemberDataUpdated);
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::SetupBindings failed to get PartyDataFactory-- delegates will not be bound."));
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

URHQueueDataFactory* URHQuickPlay::GetQueueDataFactory() const
{
    if (MyHud.IsValid())
    {
        if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
        {
            return RHLobbyHUD->GetQueueDataFactory();
        }
        else
        {
            UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::GetQueueDataFactory Warning: MyHud failed to cast to ARHLobbyHUD."));
        }
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::GetQueueDataFactory Warning: MyHud is not currently valid."));
    }
    return nullptr;
}

URHPartyManager* URHQuickPlay::GetPartyManager() const
{
    if (MyHud.IsValid())
    {
        return MyHud->GetPartyManager();
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::GetPartyDataFactory Warning: MyHud is not currently valid."));
    }
    return nullptr;
}

TArray<URH_MatchmakingQueueInfo*> URHQuickPlay::GetQueues()
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		return pQueueDataFactory->GetQueues();
	}

	return TArray<URH_MatchmakingQueueInfo*>();
}

URH_MatchmakingQueueInfo* URHQuickPlay::GetQueueInfoById(const FString& QueueId)
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

bool URHQuickPlay::IsValidQueue(const FString& QueueId) const
{
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		return (pQueueDataFactory->GetQueueInfoById(QueueId) != nullptr);
	}
	return false;
}

bool URHQuickPlay::SetCurrentlySelectedQueue(const FString& QueueId)
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

URH_MatchmakingQueueInfo* URHQuickPlay::GetCurrentlySelectedQueue() const
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

void URHQuickPlay::ReceiveMatchStatusUpdate(ERH_MatchStatus CurrentMatchStatus)
{
    SetupReadyForQueueing();
}

void URHQuickPlay::HandlePartyMemberDataUpdated(FRH_PartyMemberData PartyMember)
{
    UpdateQueuePermissions();
}

void URHQuickPlay::UpdateQueuePermissions()
{
    // check if we can queue
    if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
    {
        // we check perms but also we cannot queue if there are no queues
        bool NewCanQueue = pQueueDataFactory->CanQueue() && pQueueDataFactory->GetQueues().Num() > 0;
        if (CanCurrentlyJoinQueue != NewCanQueue)
        {
            CanCurrentlyJoinQueue = NewCanQueue;
            OnQueuePermissionChanged(CanCurrentlyJoinQueue);
        }
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::UpdateQueuePermissions failed"));
    }

    // check if we can control the queue
    if (URHPartyManager* pPartyManager = GetPartyManager())
    {
        CanControlQueue = pPartyManager->IsInParty() ? pPartyManager->IsLeader() : true;
        OnControlQueuePermissionChanged(CanControlQueue);
    }
}

FString URHQuickPlay::GetDefaultSelectedQueueId() const
{
	return DefaultSelectedQueueId;
}

void URHQuickPlay::SetupReadyForQueueing()
{
    // set the default queue
	if (URHQueueDataFactory* pQueueDataFactory = GetQueueDataFactory())
	{
		// queue Id
		FString NewSelectedQueueId = TEXT("");

		// reset to default where the queue id is not currently set, where it is set to an invalid queue id, or is a custom match
		URH_MatchmakingQueueInfo* CurrentSelectedQueue = GetCurrentlySelectedQueue();

		if (CurrentSelectedQueue != nullptr)
		{
			FRHQueueDetails QueueDetails;
			bool IsCustom = pQueueDataFactory->GetQueueDetailsByQueue(CurrentSelectedQueue, QueueDetails) && QueueDetails.IsCustom;
			if (IsCustom)
			{
				// Default assigned to the instance undefined?
				if (GetDefaultSelectedQueueId().IsEmpty())
				{
					// First one from the queue data factory
					if (pQueueDataFactory->GetQueues().Num() > 0)
					{
						NewSelectedQueueId = pQueueDataFactory->GetQueues()[0]->GetQueueId();
					}
				}
				else
				{
					// default from the quick play instance
					NewSelectedQueueId = GetDefaultSelectedQueueId();
				}
			}
			else
			{
				NewSelectedQueueId = CurrentSelectedQueue->GetQueueId();
			}
		}

		// throw an error for an invalid Id
		if (NewSelectedQueueId.IsEmpty())
		{
			UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::SetupReadyForQueueing failed to set a selected queue based on an invalid id (%s)."), *NewSelectedQueueId);
		}

		// attempt to re-set the currently selected queue
		if (!SetCurrentlySelectedQueue(NewSelectedQueueId))
		{
			if (CurrentSelectedQueue != nullptr)
			{
				UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::SetupReadyForQueueing failed, SetCurrentlySelectedQueue for Queue (%s) failed - Currently selected Queue ID is %s."), *NewSelectedQueueId, *CurrentSelectedQueue->GetQueueId());
			}
			else
			{
				UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::SetupReadyForQueueing failed, SetCurrentlySelectedQueue for Queue (%s) failed - No Currently selected Queue ID."), *NewSelectedQueueId);
			}
		}

		// trigger an update to the queue perms
		UpdateQueuePermissions();
    }
    else
    {
        UE_LOG(RallyHereStart, Warning, TEXT("URHQuickPlay::SetupReadyForQueueing failed"));
    }
}
