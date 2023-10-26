// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "GameFramework/RHGameInstance.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Subsystems/RHEventSubsystem.h"
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
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (URHEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<URHEventSubsystem>())
			{
				FTimespan Timespan = EventSubsystem->GetEventEndTime(EventTag) - EventSubsystem->GetCurrentTime();

				return (int32)Timespan.GetTotalSeconds();
			}
		}
	}
	
	return 0;
}