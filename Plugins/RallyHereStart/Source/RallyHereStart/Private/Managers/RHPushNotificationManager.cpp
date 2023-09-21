// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Kismet/BlueprintPlatformLibrary.h"
#include "Misc/ScopeExit.h"
#include "Managers/RHPushNotificationManager.h"

URHPushNotificationManager::URHPushNotificationManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	JsonPanel = TEXT("landingpanel");
}

void URHPushNotificationManager::Initialize(class URHJsonDataFactory* InJsonDataFactory)
{
	JsonDataFactory = InJsonDataFactory;
	if (JsonDataFactory)
	{
		JsonDataFactory->JsonPanelUpdated.AddDynamic(this, &URHPushNotificationManager::HandleJsonReady);
	}

	FCoreDelegates::ApplicationRegisteredForUserNotificationsDelegate.AddUObject(this, &URHPushNotificationManager::HandleRegisteredForUserNotifications);
	FCoreDelegates::ApplicationRegisteredForRemoteNotificationsDelegate.AddUObject(this, &URHPushNotificationManager::HandleRegisteredForRemoteNotifications);
}

void URHPushNotificationManager::HandleRegisteredForUserNotifications(int32 Types)
{
	bNotificationPermissionGranted = true;
	CheckNotificationState();
}

void URHPushNotificationManager::HandleRegisteredForRemoteNotifications(TArray<uint8> InToken)
{
	bNotificationPermissionGranted = true;
	CheckNotificationState();
}

void URHPushNotificationManager::SetState(ERHPushNotificationState NewState)
{
	State = NewState;
	CheckNotificationState();
}

void URHPushNotificationManager::CheckNotificationState()
{
#if !UE_BUILD_SHIPPING
	// Allow a command line argument to test push notification system logic
	if (State == ERHPushNotificationState::WaitForNotificationData && 
		FParse::Param(FCommandLine::Get(), TEXT("FakePushNotification")))
	{
		URHJsonPushNotification* Notification = NewObject<URHJsonPushNotification>();
		if (Notification)
		{
			Notification->StartTime = FDateTime::UtcNow();
			Notification->StartTime += FTimespan(0, 1, 0);
			Notification->Header = TEXT("Test Notification Title");
			Notification->Body = TEXT("Test Notification Body");
			Notifications.Add(Notification);
		}
	}
#endif

	if (State == ERHPushNotificationState::WaitForNotificationData)
	{
		if (Notifications.Num() > 0)
		{
			SetState(ERHPushNotificationState::StartRequestPermissions);
		}
	}
	else if (State == ERHPushNotificationState::StartRequestPermissions)
	{
		if (bNotificationPermissionGranted)
		{
			SetState(ERHPushNotificationState::ScheduleNotifications);
		}
		else
		{
			FPlatformMisc::RegisterForRemoteNotifications();
			SetState(ERHPushNotificationState::RequestedPermissions);
		}
	}
	else if (State == ERHPushNotificationState::RequestedPermissions)
	{
		if(bNotificationPermissionGranted)
		{
			SetState(ERHPushNotificationState::ScheduleNotifications);
		}
	}
	else if (State == ERHPushNotificationState::ScheduleNotifications)
	{
		ScheduleNotifications();
		SetState(ERHPushNotificationState::Complete);
	}
}

void URHPushNotificationManager::ScheduleNotifications()
{
	UBlueprintPlatformLibrary::ClearAllLocalNotifications();

	for (const URHJsonPushNotification* Notification : Notifications)
	{
		if (!Notification)
		{
			continue;
		}

		if (Notification->StartTime.GetTicks() > 0)
		{
			continue;
		}

		FDateTime StartTime = Notification->StartTime;
		FTimespan TimeDifference = FDateTime::UtcNow() - FDateTime::Now();
		StartTime += TimeDifference;

		UBlueprintPlatformLibrary::ScheduleLocalNotificationAtTime(
			StartTime, true,
			FText::FromString(Notification->Header),
			FText::FromString(Notification->Body),
			FText(),
			Notification->ActivationEvent);
	}
}

void URHPushNotificationManager::HandleJsonReady(const FString& JsonName)
{
	ON_SCOPE_EXIT
	{
		CheckNotificationState();
	};

	if (!JsonDataFactory)
	{
		return;
	}

	if (JsonName != JsonPanel)
	{
		return;
	}

	TSharedPtr<FJsonObject> LandingPanelJson = JsonDataFactory->GetJsonPanelByName(JsonPanel);
	if (!LandingPanelJson.IsValid())
	{
		return;
	}

	const TSharedPtr<FJsonObject>* PushNotificationsObj;
	if (!LandingPanelJson.Get()->TryGetObjectField(TEXT("pushNotifications"), PushNotificationsObj))
	{
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* ContentArray;
	if (!(*PushNotificationsObj)->TryGetArrayField(TEXT("content"), ContentArray))
	{
		return;
	}

	State = ERHPushNotificationState::WaitForNotificationData;
	Notifications.Empty(ContentArray->Num());

	for (TSharedPtr<FJsonValue> ContentValue : *ContentArray)
	{
		const TSharedPtr<FJsonObject>* ContentObj;
		if (!ContentValue->TryGetObject(ContentObj))
		{
			continue;
		}

		URHJsonPushNotification* Notification = NewObject<URHJsonPushNotification>();
		if (!Notification)
		{
			continue;
		}

		JsonDataFactory->LoadData(Notification, ContentObj);

		// Switch out the start time and end time.
		// URHJsonDataFactory::ShouldShow treats the "EndTime" the way we want to handle
		// Push Notification start time.
		Swap(Notification->StartTime, Notification->EndTime);

		JsonDataFactory->CheckShouldShowForPlayer(Notification, JsonDataFactory->GetLocalPlayerInfo(), FOnShouldShow::CreateUObject(this, &URHPushNotificationManager::OnCheckShouldShowResponse, Notification, ContentObj));
	}
}

void URHPushNotificationManager::OnCheckShouldShowResponse(bool bShouldShow, URHJsonPushNotification* Notification, const TSharedPtr<FJsonObject>* ContentObj)
{
	if (bShouldShow)
	{
		Notification->StartTime = Notification->EndTime; // Make Start+End time the same for ease of understanding

		const TSharedPtr<FJsonObject>* ObjectField;
		if ((*ContentObj)->TryGetObjectField(TEXT("headerText"), ObjectField))
		{
			Notification->Header = URHJsonDataFactory::GetLocalizedStringFromObject(ObjectField);
		}
		if ((*ContentObj)->TryGetObjectField(TEXT("bodyText"), ObjectField))
		{
			Notification->Body = URHJsonDataFactory::GetLocalizedStringFromObject(ObjectField);
		}

		(*ContentObj)->TryGetStringField(TEXT("activationEvent"), Notification->ActivationEvent);

		Notifications.Add(Notification);
	}
}
