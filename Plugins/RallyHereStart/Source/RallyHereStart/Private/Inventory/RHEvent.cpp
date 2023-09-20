// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "GameFramework/RHGameInstance.h"
#include "Managers/RHStoreItemHelper.h"
#include "Managers/RHEventManager.h"
#include "RH_PlayerInfoSubsystem.h"
#include "Inventory/RHEvent.h"

URHEvent::URHEvent(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	IsOwnableInventoryItem = false;

	static const FGameplayTag EventCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::EventCollectionName);
	CollectionContainer.AddTag(EventCollectionTag);
}

int32 URHEvent::GetRemainingSeconds(const UObject* WorldContextObject) const
{
	UWorld* const World = GEngine != nullptr ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;

	if (World != nullptr)
	{
		if (URHGameInstance* GameInstance = Cast<URHGameInstance>(World->GetGameInstance()))
		{
			if (URHEventManager* EventManager = GameInstance->GetEventManager())
			{
				FTimespan Timespan = EventManager->GetEventEndTime(EventTag) - EventManager->GetCurrentTime();

				return (int32)Timespan.GetTotalSeconds();
			}
		}
	}
	
	return 0;
}