#pragma once
#include "Engine/DataTable.h"
#include "Inventory/RHEvent.h"
#include "RH_ConfigSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RHEventSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FRHEventData : public FTableRowBase
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Event Data")
	TSoftObjectPtr<URHEvent> DataObject;

	FRHEventData()
    {
    }
};

UCLASS(Config = Game)
class RALLYHERESTART_API URHEventSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Finds the first active event of the given type
	template <typename T>
	T* GetActiveEvent() const
	{
		if (EventsDataDT != nullptr)
		{
			for (FName EventTag : EventsDataDT->GetRowNames())
			{
				// Only check active events so we don't try and load everything
				if (IsEventActive(EventTag))
				{
					if (FRHEventData* EventRow = EventsDataDT->FindRow<FRHEventData>(EventTag, "Load Active Event To Check"))
					{
						// Only load the event if it potentially could match what we need
						// Loading active events that are not needed isn't the end of the world something else will probably be looking for it 
						URHEvent* Event = EventRow->DataObject.IsValid() ? EventRow->DataObject.Get() : EventRow->DataObject.LoadSynchronous();

						if (Event != nullptr)
						{
							if (T* RetEvent = Cast<T>(Event))
							{
								RetEvent->EventTag = EventTag;
								return RetEvent;
							}
						}
					}
				}
			}
		}
		return nullptr;
	}

	// When the player gets logged in check for all active events and request their needed vendors
	void OnLoginPlayerChanged(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintCallable)
	URHEvent* GetEventByTag(FName EventTag) const;

	// Returns whether we are within the event's timeframe for it to be active
	bool IsEventActive(FName EventTag) const;

	// Helper function that just checks if we are past the start date for an event
	bool IsEventPastStartDate(FName EventTag) const;

	// Helper function that just checks if we are before the end date for an event
	bool IsEventBeforeEndDate(FName EventTag) const;

	// Gets the server start time of the event
	FDateTime GetEventStartTime(FName EventTag) const;

	// Gets the server end time of the event
	FDateTime GetEventEndTime(FName EventTag) const;

	// Gets the current server time
	FDateTime GetCurrentTime() const;

protected:
	/** Path to the DataTable to load, configurable per game. If empty, it will not spawn one */
	UPROPERTY(Config)
	FSoftObjectPath EventSubsystemDataTableClassName;

	// Table of all available Context Actions
	UPROPERTY(Transient)
	UDataTable* EventsDataDT;

	bool bEventsInitialized;

	URH_ConfigSubsystem* GetConfigSubsystem() const;
};