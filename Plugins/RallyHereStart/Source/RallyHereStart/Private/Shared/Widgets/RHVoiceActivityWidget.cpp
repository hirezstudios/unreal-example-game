// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Widgets/RHVoiceActivityWidget.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Managers/RHPartyManager.h"
#include "RHUIBlueprintFunctionLibrary.h"
#include "GameFramework/RHGameState.h"

void URHVoiceActivityWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// #RHTODO - Voice
	// 
	//if (IPComVoiceInterface* const pVoiceInterface = pcomGetVoice())
	//{
	//	ParticipantAddedHandle = pVoiceInterface->EventAfterParticipantAdded.AddUObject(this, &URHVoiceActivityWidget::HandleVoiceInterfaceParticipantAdded);
	//	ParticipantRemovedHandle = pVoiceInterface->EventBeforeBeforeParticipantRemoved.AddUObject(this, &URHVoiceActivityWidget::HandleVoiceInterfaceParticipantRemoved);
	//	ParticipantUpdatedHandle = pVoiceInterface->EventAfterParticipantUpdated.AddUObject(this, &URHVoiceActivityWidget::HandleVoiceInterfaceParticipantUpdated);
	//	AudioStateChangedHandle = pVoiceInterface->EventAudioStateChanged.AddUObject(this, &URHVoiceActivityWidget::HandleAudioStateChanged);
	//}
}

void URHVoiceActivityWidget::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();

	// #RHTODO - Voice

	//if (IPComVoiceInterface* const pVoiceInterface = pcomGetVoice())
	//{
	//	pVoiceInterface->EventAfterParticipantAdded.Remove(ParticipantAddedHandle);
	//	pVoiceInterface->EventBeforeBeforeParticipantRemoved.Remove(ParticipantRemovedHandle);
	//	pVoiceInterface->EventAfterParticipantUpdated.Remove(ParticipantUpdatedHandle);
	//	pVoiceInterface->EventAudioStateChanged.Remove(AudioStateChangedHandle);
	//}
}

FString URHVoiceActivityWidget::GetVoiceIdByPlayerUuid(const FGuid& PlayerId) const
{
	return PlayerId.ToString();
}

FGuid URHVoiceActivityWidget::GetPlayerUuidByVoiceId(const FString& VoiceId) const
{
	return FGuid(*VoiceId);
}

class ARHPlayerState* URHVoiceActivityWidget::GetPlayerStateByVoiceId(const FString& VoiceId) const
{
	if (UWorld* const World = GetWorld())
	{
		if (ARHGameState* GameState = World->GetGameState<ARHGameState>())
		{
			const FGuid PlayerUuid = GetPlayerUuidByVoiceId(VoiceId);
			for (APlayerState* pPlayerState : GameState->PlayerArray)
			{
				ARHPlayerState* pRHPlayerState = Cast<ARHPlayerState>(pPlayerState);
				if (pRHPlayerState != nullptr && pRHPlayerState->GetRHPlayerUuid() == PlayerUuid)
				{
					return pRHPlayerState;
				}
			}
		}
	}

	return nullptr;
}

/* #RHTODO - Voice
void URHVoiceActivityWidget::HandleVoiceInterfaceParticipantAdded(const IPComVoiceInterface::FParticipant& ParticipantAdded)
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHVoiceActivity::HandleVoiceInterfaceParticipantAdded - Participant name: %s on channel: %s"), *ParticipantAdded.m_AccountName, *ParticipantAdded.m_ChannelName);
	OnVoiceParticipantAdded(ParticipantAdded.m_AccountName);
	VoiceParticipantAdded.Broadcast(ParticipantAdded.m_AccountName);
}

void URHVoiceActivityWidget::HandleVoiceInterfaceParticipantRemoved(const IPComVoiceInterface::FParticipant& ParticipantRemoved)
{
    UE_LOG(RallyHereStart, Verbose, TEXT("URHVoiceActivity::HandleVoiceInterfaceParticipantRemoved - Participant name: %s on channel: %s"), *ParticipantRemoved.m_AccountName, *ParticipantRemoved.m_ChannelName);
    OnVoiceParticipantRemoved(ParticipantRemoved.m_AccountName);
	VoiceParticipantRemoved.Broadcast(ParticipantRemoved.m_AccountName);
}


void URHVoiceActivityWidget::HandleVoiceInterfaceParticipantUpdated(const IPComVoiceInterface::FParticipant& ParticipantUpdated)
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHVoiceActivity::HandleVoiceInterfaceParticipantUpdated - Participant name: %s on channel: %s"), *ParticipantUpdated.m_AccountName, *ParticipantUpdated.m_ChannelName);
	OnVoiceParticipantUpdated(ParticipantUpdated.m_AccountName, ParticipantUpdated.m_bSpeechDetected, ParticipantUpdated.m_bIsMuted);
	VoiceParticipantUpdated.Broadcast(ParticipantUpdated.m_AccountName, ParticipantUpdated.m_bSpeechDetected, ParticipantUpdated.m_bIsMuted);
}

void URHVoiceActivityWidget::HandleAudioStateChanged(const IPComVoiceInterface::FAudioState& AudioState)
{
	ERHVoiceActivityAudioState StateToBroadcast;
	switch (AudioState.m_State)
	{
	case IPComVoiceInterface::FAudioState::EState::Connected:
		UE_LOG(RallyHereStart, Verbose, TEXT("URHVoiceActivityWidget::HandleAudioStateChanged Voice Activity Connected"));
		StateToBroadcast = ERHVoiceActivityAudioState::Connected;
		break;
	case IPComVoiceInterface::FAudioState::EState::Connecting:
		UE_LOG(RallyHereStart, Verbose, TEXT("URHVoiceActivityWidget::HandleAudioStateChanged Voice Activity Connecting"));
		StateToBroadcast = ERHVoiceActivityAudioState::Connecting;
		break;
	case IPComVoiceInterface::FAudioState::EState::Disconnecting:
		UE_LOG(RallyHereStart, Verbose, TEXT("URHVoiceActivityWidget::HandleAudioStateChanged Voice Activity Disconnecting"));
		StateToBroadcast = ERHVoiceActivityAudioState::Disconnecting;
		break;
	case IPComVoiceInterface::FAudioState::EState::Disconnected:
	default:
		UE_LOG(RallyHereStart, Verbose, TEXT("URHVoiceActivityWidget::HandleAudioStateChanged Voice Activity Disconnected"));
		StateToBroadcast = ERHVoiceActivityAudioState::Disconnected;
		break;
	}
	VoiceAudioStateChange.Broadcast(StateToBroadcast);
}
*/

URH_PlayerInfo* URHVoiceActivityWidget::GetPlayerInfoByVoiceId(const FString& VoiceId) const
{
	// Look for the player in party data
	if (MyHud.IsValid())
	{
		if (URHPartyManager* pPartyData = MyHud->GetPartyManager())
		{
			URH_PlayerInfo* PartyMemberPlayerInfo = pPartyData->GetPartyMemberByID(GetPlayerUuidByVoiceId(VoiceId)).PlayerData;
			if (PartyMemberPlayerInfo != nullptr)
			{
				return PartyMemberPlayerInfo;
			}
		}
	}

	// Look for the player in game state players
	if (ARHPlayerState* PlayerState = GetPlayerStateByVoiceId(VoiceId))
	{
		return PlayerState->GetPlayerInfo(MyHud.Get());
	}

	return nullptr;
}