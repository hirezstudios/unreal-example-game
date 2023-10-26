
#include "Subsystems/RHStoreSubsystem.h"
#include "RHUIBlueprintFunctionLibrary.h"
#include "GameFramework/RHGameInstance.h"
#include "RH_GameInstanceSubsystem.h"
#include "Subsystems/RHEventSubsystem.h"

bool URHEventSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}

void URHEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (EventSubsystemDataTableClassName.ToString().Len() > 0)
	{
		EventsDataDT = LoadObject<UDataTable>(nullptr, *EventSubsystemDataTableClassName.ToString(), nullptr, LOAD_None, nullptr);
	}

	bEventsInitialized = false;

	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		GameInstance->OnLocalPlayerLoginChanged.AddUObject(this, &URHEventSubsystem::OnLoginPlayerChanged);
	}
}

void URHEventSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (URHGameInstance* GameInstance = Cast<URHGameInstance>(GetGameInstance()))
	{
		GameInstance->OnLocalPlayerLoginChanged.RemoveAll(this);
	}
}

void URHEventSubsystem::OnLoginPlayerChanged(ULocalPlayer* LocalPlayer)
{
	if (!bEventsInitialized)
	{
		TArray<int32> VendorIdsToRequest;

		if (EventsDataDT != nullptr)
		{
			for (FName EventTag : EventsDataDT->GetRowNames())
			{
				// Only check active events so we don't try and load everything
				if (IsEventActive(EventTag))
				{
					if (FRHEventData* EventRow = EventsDataDT->FindRow<FRHEventData>(EventTag, "Load Activity to get its required loot table Ids"))
					{
						if (URHEvent* Event = EventRow->DataObject.LoadSynchronous())
						{
							Event->AppendRequiredLootTableIds(VendorIdsToRequest);
						}
					}
				}
			}
		}

		if (VendorIdsToRequest.Num() > 0)
		{
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
				{
					StoreSubsystem->RequestVendorData(VendorIdsToRequest);
				}
			}
		}

		bEventsInitialized = true;
	}
}

URHEvent* URHEventSubsystem::GetEventByTag(FName EventTag) const
{
	if (EventsDataDT != nullptr)
	{
		if (FRHEventData* EventRow = EventsDataDT->FindRow<FRHEventData>(EventTag, "Load Requested Event"))
		{
			URHEvent* Event = EventRow->DataObject.IsValid() ? EventRow->DataObject.Get() : EventRow->DataObject.LoadSynchronous();
			
			if (Event != nullptr)
			{
				Event->EventTag = EventTag;
				return Event;
			}
		}
	}

	return nullptr;
}

bool URHEventSubsystem::IsEventActive(FName EventTag) const
{
	if (IsEventPastStartDate(EventTag) && IsEventBeforeEndDate(EventTag))
	{
		return true;
	}

	return false;
}

bool URHEventSubsystem::IsEventPastStartDate(FName EventTag) const
{
	FDateTime StartTime = GetEventStartTime(EventTag);
	FDateTime CurrentTime = GetCurrentTime();

	if (StartTime.GetTicks() > 0 && CurrentTime.GetTicks() > 0)
	{
		if (StartTime <= CurrentTime)
		{
			return true;
		}
	}

	return false;
}

bool URHEventSubsystem::IsEventBeforeEndDate(FName EventTag) const
{
	FDateTime EndTime = GetEventEndTime(EventTag);
	FDateTime CurrentTime = GetCurrentTime();

	if (EndTime.GetTicks() > 0 && CurrentTime.GetTicks() > 0)
	{
		if (CurrentTime < EndTime)
		{
			return true;
		}
	}

	return false;
}

FDateTime URHEventSubsystem::GetCurrentTime() const
{
	return FDateTime::UtcNow();
}

FDateTime URHEventSubsystem::GetEventStartTime(FName EventTag) const
{
	FDateTime TimeStart;
	FString TimeStartStr;
	if (URH_ConfigSubsystem* ConfigSubsystem = GetConfigSubsystem())
	{
		FString ConfigEntryNameStartTime = EventTag.ToString() + ".StartTime";
		ConfigSubsystem->GetAppSetting(ConfigEntryNameStartTime, TimeStartStr);
		RallyHereAPI::ParseDateTime(*TimeStartStr, TimeStart);
	}
	
	return TimeStart;
}

FDateTime URHEventSubsystem::GetEventEndTime(FName EventTag) const
{
	FDateTime TimeEnd;
	FString TimeEndStr;
	if (URH_ConfigSubsystem* ConfigSubsystem = GetConfigSubsystem())
	{
		FString ConfigEntryNameEndTime = EventTag.ToString() + ".EndTime";
		ConfigSubsystem->GetAppSetting(ConfigEntryNameEndTime, TimeEndStr);
		RallyHereAPI::ParseDateTime(*TimeEndStr, TimeEnd);
	}
	
	return TimeEnd;
}

URH_ConfigSubsystem* URHEventSubsystem::GetConfigSubsystem() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (URH_GameInstanceSubsystem* pGISS = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
			{
				return pGISS->GetConfigSubsystem();
			}
		}
	}

	return nullptr;
}