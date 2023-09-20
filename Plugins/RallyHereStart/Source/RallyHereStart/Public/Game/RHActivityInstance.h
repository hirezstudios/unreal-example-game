// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RHActivity.h"
#include "Player/Controllers/RHPlayerController.h"
#include "RHActivityInstance.generated.h"

UCLASS(Abstract)
class RALLYHERESTART_API URHActivityInstance : public UObject
{
	GENERATED_BODY()
	
public:
    URHActivityInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//This will be called when the instance creates an activity instance to track for a player
	void InitializeActivity(URHActivity* InActivity, const ARHPlayerController* InController);

	// Used by subclasses to setup listeners to any events in the game that need to be tracked by the activity
	virtual void InitializeTracking() { }

	// Finalizes the total progress the player earns from the activity, and commits it to the core
	// Returns a data blob that can be used by the post match screen to display progress
	virtual int32 FinalizeRewards() { return 0; }

protected:
	// This was the controller that was used to initialize this activity
	UPROPERTY(BlueprintReadOnly, Transient)
	TWeakObjectPtr<const ARHPlayerController> OwningController;

	// This is the activity that was used to create this ActivityInstance.
	UPROPERTY(BlueprintReadOnly, Transient)
	URHActivity* Activity;

	// This is the progress the player has at the start of the match
	int32 InitialProgress;
};
