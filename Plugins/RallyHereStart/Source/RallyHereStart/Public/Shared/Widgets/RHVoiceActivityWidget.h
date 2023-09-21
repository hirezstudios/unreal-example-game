// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHVoiceActivityWidget.generated.h"

class ARHPlayerState;

UENUM(BlueprintType)
enum class ERHVoiceActivityAudioState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	Disconnecting
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoiceAccountNamePairsUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceParticipantAdded, const FString&, AccountId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceParticipantRemoved, const FString&, AccountId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVoiceParticipantUpdated, const FString&, AccountId, bool, bIsTalking, bool, bIsMuted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceAudioStateChange, ERHVoiceActivityAudioState, AudioState);
/**
 * Voice Activity Widget
 */
UCLASS()
class RALLYHERESTART_API URHVoiceActivityWidget : public URHWidget
{
    GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void UninitializeWidget_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Voice Activity")
	FString GetVoiceIdByPlayerUuid(const FGuid& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Voice Activity")
	FGuid GetPlayerUuidByVoiceId(const FString& VoiceId) const;

	UFUNCTION(BlueprintPure, Category = "Voice Activity")
	class ARHPlayerState* GetPlayerStateByVoiceId(const FString& VoiceId) const;

	UFUNCTION(BlueprintPure, Category = "Voice Activity")
	URH_PlayerInfo* GetPlayerInfoByVoiceId(const FString& VoiceId) const;

	// Call GetLastKnownDisplayNameAsync on the PlayerInfo if their name is needed.

protected:
    UPROPERTY(BlueprintAssignable, Category = "Voice Activity")
    FVoiceAccountNamePairsUpdated VoiceAccountNamePairsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Voice Activity")
    FOnVoiceParticipantAdded VoiceParticipantAdded;

    UPROPERTY(BlueprintAssignable, Category = "Voice Activity")
    FOnVoiceParticipantRemoved VoiceParticipantRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Voice Activity")
    FOnVoiceParticipantUpdated VoiceParticipantUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Voice Activity")
    FOnVoiceAudioStateChange VoiceAudioStateChange;

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice Activity")
	void OnVoiceParticipantAdded(const FString& AccountId);

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice Activity")
    void OnVoiceParticipantRemoved(const FString& AccountId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice Activity")
	void OnVoiceParticipantUpdated(const FString& AccountId, bool bIsTalking, bool bIsMuted);

private:

	FDelegateHandle ParticipantAddedHandle;
	FDelegateHandle ParticipantUpdatedHandle;
	FDelegateHandle ParticipantRemovedHandle;
	FDelegateHandle AudioStateChangedHandle;
};
