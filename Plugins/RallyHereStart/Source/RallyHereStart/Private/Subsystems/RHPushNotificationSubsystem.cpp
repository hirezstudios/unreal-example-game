// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Kismet/BlueprintPlatformLibrary.h"
#include "Misc/ScopeExit.h"
#include "Subsystems/RHPushNotificationSubsystem.h"

bool URHPushNotificationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}

void URHPushNotificationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<URHNewsSubsystem>();

	Super::Initialize(Collection);

	JsonPanel = TEXT("landingpanel");

	UGameInstance* GameInstance = GetGameInstance();
	URHNewsSubsystem* NewsSubsystem = GameInstance != nullptr ? GameInstance->GetSubsystem<URHNewsSubsystem>() : nullptr;

	if (NewsSubsystem)
	{
		NewsSubsystem->JsonPanelUpdated.AddDynamic(this, &URHPushNotificationSubsystem::HandleJsonReady);
	}

	FCoreDelegates::ApplicationRegisteredForUserNotificationsDelegate.AddUObject(this, &URHPushNotificationSubsystem::HandleRegisteredForUserNotifications);
	FCoreDelegates::ApplicationRegisteredForRemoteNotificationsDelegate.AddUObject(this, &URHPushNotificationSubsystem::HandleRegisteredForRemoteNotifications);
}

void URHPushNotificationSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UGameInstance* GameInstance = GetGameInstance();
	URHNewsSubsystem* NewsSubsystem = GameInstance != nullptr ? GameInstance->GetSubsystem<URHNewsSubsystem>() : nullptr;

	if (NewsSubsystem)
	{
		NewsSubsystem->JsonPanelUpdated.RemoveAll(this);
	}
}

void URHPushNotificationSubsystem::HandleRegisteredForUserNotifications(int32 Types)
{
	bNotificationPermissionGranted = true;
	CheckNotificationState();
}

void URHPushNotificationSubsystem::HandleRegisteredForRemoteNotifications(TArray<uint8> InToken)
{
	bNotificationPermissionGranted = true;
	CheckNotificationState();
}

void URHPushNotificationSubsystem::SetState(ERHPushNotificationState NewState)
{
	State = NewState;
	CheckNotificationState();
}

void URHPushNotificationSubsystem::CheckNotificationState()
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

void URHPushNotificationSubsystem::ScheduleNotifications()
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

void URHPushNotificationSubsystem::HandleJsonReady(const FString& JsonName)
{
	ON_SCOPE_EXIT
	{
		CheckNotificationState();
	};

	UGameInstance* GameInstance = GetGameInstance();
	URHNewsSubsystem* NewsSubsystem = GameInstance != nullptr ? GameInstance->GetSubsystem<URHNewsSubsystem>() : nullptr;

	if (!NewsSubsystem)
	{
		return;
	}

	if (JsonName != JsonPanel)
	{
		return;
	}

	TSharedPtr<FJsonObject> LandingPanelJson = NewsSubsystem->GetJsonPanelByName(JsonPanel);
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

		NewsSubsystem->LoadData(Notification, ContentObj);

		// Switch out the start time and end time.
		// URHNewsSubsystem::ShouldShow treats the "EndTime" the way we want to handle
		// Push Notification start time.
		Swap(Notification->StartTime, Notification->EndTime);

		// #RHTODO: This is based on a local player, but this is from Game Instance, this might need some re-thinking
		NewsSubsystem->CheckShouldShowForPlayer(Notification, nullptr, FOnShouldShow::CreateUObject(this, &URHPushNotificationSubsystem::OnCheckShouldShowResponse, Notification, ContentObj));
	}
}

void URHPushNotificationSubsystem::OnCheckShouldShowResponse(bool bShouldShow, URHJsonPushNotification* Notification, const TSharedPtr<FJsonObject>* ContentObj)
{
	if (bShouldShow)
	{
		Notification->StartTime = Notification->EndTime; // Make Start+End time the same for ease of understanding

		const TSharedPtr<FJsonObject>* ObjectField;
		if ((*ContentObj)->TryGetObjectField(TEXT("headerText"), ObjectField))
		{
			Notification->Header = URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField);
		}
		if ((*ContentObj)->TryGetObjectField(TEXT("bodyText"), ObjectField))
		{
			Notification->Body = URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField);
		}

		(*ContentObj)->TryGetStringField(TEXT("activationEvent"), Notification->ActivationEvent);

		Notifications.Add(Notification);
	}
}
