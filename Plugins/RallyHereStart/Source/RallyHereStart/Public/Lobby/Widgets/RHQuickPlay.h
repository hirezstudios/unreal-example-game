// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "Managers/RHPartyManager.h"
#include "RHQuickPlay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSelectedQueueChanged, URH_MatchmakingQueueInfo*, SelectedQueue);

/**
 * Quick play queue and game mode selection widget
 */
/**
* 
* Marked for removal, and intended to be replaced by RHQueueWidgetBase
* In order to unblock UI design work, this replacement/refactor is delayed.
* - CGS 03.30.2020
*
**/
UCLASS()
class RALLYHERESTART_API URHQuickPlay : public URHWidget
{

	GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Quick Queue Widget")
    void                                             SetupBindings();

protected:
    // get the queue data factory
    UFUNCTION()
    URHQueueDataFactory*                          GetQueueDataFactory() const;

    // get the party data factory
    UFUNCTION()
    URHPartyManager*								GetPartyManager() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Quick Queue Widget")
	FSelectedQueueChanged							 OnSelectedQueueChanged;

    UPROPERTY(BlueprintReadOnly, Category = "Quick Queue Widget")
    bool                                             CanCurrentlyJoinQueue;

    UPROPERTY(BlueprintReadOnly, Category = "Quick Queue Widget")
    bool                                             CanControlQueue;

    // Gets the current game modes
    UFUNCTION(BlueprintCallable, Category = "Quick Queue Widget")
    TArray<URH_MatchmakingQueueInfo*>                GetQueues();

	UFUNCTION(BlueprintCallable, Category = "Quick Queue Widget")
	URH_MatchmakingQueueInfo*						 GetQueueInfoById(const FString& QueueId);

    // get the current selected queue
    UFUNCTION(BlueprintCallable, Category = "Quick Queue Widget")
	URH_MatchmakingQueueInfo*						 GetCurrentlySelectedQueue() const;

	UFUNCTION(BlueprintPure, Category = "Quick Queue Widget")
	bool                                             IsValidQueue(const FString& QueueId) const;

    // Set the current selected queue
    UFUNCTION(BlueprintCallable, Category = "Quick Queue Widget")
    bool                                             SetCurrentlySelectedQueue(const FString& QueueId);

    // blueprint event triggered when user queuing permissions change
    UFUNCTION(BlueprintImplementableEvent, Category = "Quick Queue Widget")
    void                                             OnQueuePermissionChanged(bool CanQueue);

    // blueprint event triggered when user queuing control permissions change
    UFUNCTION(BlueprintImplementableEvent, Category = "Quick Queue Widget")
    void                                             OnControlQueuePermissionChanged(bool CanControl);

protected:
    UPROPERTY(EditAnywhere, Category = "Quick Queue Widget")
    FString                                          DefaultSelectedQueueId;

	UFUNCTION(BlueprintPure)
	FString											 GetDefaultSelectedQueueId() const;

	UFUNCTION()
    void                                             ReceiveMatchStatusUpdate(ERH_MatchStatus CurrentMatchStatus);

    UFUNCTION()
    void                                             HandlePartyMemberDataUpdated(FRH_PartyMemberData PartyMember);

    UFUNCTION()
    void                                             UpdateQueuePermissions();

    UFUNCTION()
    void                                             SetupReadyForQueueing();

private:
    UPROPERTY()
    bool                                             ReadyForQueueing;
};
