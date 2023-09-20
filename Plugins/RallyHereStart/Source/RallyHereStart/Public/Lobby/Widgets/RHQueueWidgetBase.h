// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "Managers/RHPartyManager.h"
#include "RHQueueWidgetBase.generated.h"

class URHQueueDataFactory;

/**
 * Widget base class for queue related widget
 */
UCLASS()
class RALLYHERESTART_API URHQueueWidgetBase : public URHWidget
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Queue Widget")
    void                                             SetupBindings();

	/**
	* Internal helpers
	**/
protected:
    // get the queue data factory
    UFUNCTION(BlueprintPure, Category = "Queue Widget")
    URHQueueDataFactory*					GetQueueDataFactory() const;

    // get the party data factory
    UFUNCTION()
    URHPartyManager*                          GetPartyManager() const;

	/**
	* State change updates
	**/
protected:
	// blueprint event triggered when user queuing permissions change
	UFUNCTION(BlueprintNativeEvent, Category = "Queue Widget")
	void                                             OnQueuePermissionUpdate(bool CanQueue);

	// blueprint event triggered when user queuing control permissions change
	UFUNCTION(BlueprintNativeEvent, Category = "Queue Widget")
	void                                             OnControlQueuePermissionUpdate(bool CanControl);

	// blueprint event triggered when selected queue id has changed
	UFUNCTION(BlueprintNativeEvent, Category = "Queue Widget")
	void                                             OnSelectedQueueUpdate(URH_MatchmakingQueueInfo* CurrentSelectedQueue);

	// blueprint event triggered when user queuing control permissions change
	UFUNCTION(BlueprintNativeEvent, Category = "Queue Widget")
	void                                             OnQueueStateUpdate(ERH_MatchStatus CurrentMatchStatus);

protected: 
	UFUNCTION()
	void                                             ReceiveMatchStatusUpdate(ERH_MatchStatus CurrentMatchStatus);

	UFUNCTION()
	void                                             HandlePartyMemberDataUpdated(FRH_PartyMemberData PartyMember);

	UFUNCTION()
	void                                             HandlePartyMemberRemoved(const FGuid& PartyMemberId);

	UFUNCTION()
	void                                             UpdateQueuePermissions();

	UFUNCTION()
	virtual void                                     UpdateQueueSelection();

	UFUNCTION()
	void                                             SetupReadyForQueueing();

	UFUNCTION()
	void                                             HandleSelectedQueueIdSet();
	
	UFUNCTION()
	void                                             HandleMatchStatusUpdate(ERH_MatchStatus MatchStatus);


	void                                             ApplyQueueChangeHelper(URHQueueDataFactory* pQueueDataFactory, const FString& QueueId);

	UFUNCTION()
	void											 HandleConfirmLeaveQueue();

	/**
	* API
	**/
public:
    // Gets the currently available queues
    UFUNCTION(BlueprintCallable, Category = "Queue Widget")
    TArray<URH_MatchmakingQueueInfo*>                GetQueues();

	// Gets queue info by Queue Id
	UFUNCTION(BlueprintCallable, Category = "Queue Widget")
	URH_MatchmakingQueueInfo*						 GetQueueInfoById(const FString& QueueId);

	// returns the queue permissions
	UFUNCTION(BlueprintCallable, Category = "Queue Widget")
	void                                             GetQueuePermissions(bool& CanControl, bool& CanQueue);

	// checks the validity of a queue by Queue Id
	UFUNCTION(BlueprintPure, Category = "Queue Widget")
	bool                                             IsValidQueue(const FString& QueueId) const;

	// get Queue Data for the currently selected Queue (wraps QueueDataFactory call)
	UFUNCTION(BlueprintCallable, Category = "Queue Widget")
	URH_MatchmakingQueueInfo*                         GetCurrentlySelectedQueue() const;

    // Set the current selected queue
    UFUNCTION(BlueprintCallable, Category = "Queue Widget")
    bool                                             SetCurrentlySelectedQueue(const FString& QueueId);

    // trigger a join queue request
    UFUNCTION(BlueprintCallable, Category = "Queue Widget")
    bool                                             UIX_AttemptJoinSelectedQueue();

    // trigger a cancel queue request
    UFUNCTION(BlueprintCallable, Category = "Queue Widget")
    bool                                             UIX_AttemptCancelQueue();

	// trigger a rejoin match attempt
	UFUNCTION(BlueprintCallable, Category = "Queue Widget")
	bool                                             UIX_AttemptRejoinMatch();

	// trigger a leave queue attempt, unblocked by leader status
	UFUNCTION(BlueprintCallable, Category = "Queue Widget")
	bool                                             UIX_AttemptLeaveMatch();
};
