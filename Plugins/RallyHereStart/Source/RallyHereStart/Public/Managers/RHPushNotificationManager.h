#pragma once
#include "Managers/RHJsonDataFactory.h"
#include "RHPushNotificationManager.generated.h"

UCLASS(BlueprintType)
class RALLYHERESTART_API URHJsonPushNotification : public URHJsonData
{
	GENERATED_BODY()

public:
	// Header text for the panel
	UPROPERTY(BlueprintReadOnly)
	FString Header;

	// Sub-header text for panel
	UPROPERTY(BlueprintReadOnly)
	FString Body;

	// String passed into the game upon launching
	// from notification
	UPROPERTY(BlueprintReadOnly)
	FString ActivationEvent;
};

UENUM()
enum class ERHPushNotificationState : uint8
{
	WaitForNotificationData,
	StartRequestPermissions,
	RequestedPermissions,
	ScheduleNotifications,
	Complete
};

UCLASS(Config = Game)
class RALLYHERESTART_API URHPushNotificationManager : public UObject
{
    GENERATED_BODY()

public:
	URHPushNotificationManager(const FObjectInitializer& ObjectInitializer);

	void Initialize(class URHJsonDataFactory* InJsonDataFactory);

	void CheckNotificationState();

	void SetState(ERHPushNotificationState NewState);

	void ScheduleNotifications();

	UFUNCTION()
	void HandleRegisteredForUserNotifications(int32 Types);

	UFUNCTION()
	void HandleRegisteredForRemoteNotifications(TArray<uint8> InToken);

	UFUNCTION()
	void HandleJsonReady(const FString& JsonName);

	UPROPERTY(Transient)
	class URHJsonDataFactory* JsonDataFactory;

	UPROPERTY(config)
	FString JsonPanel;

	UPROPERTY(Transient)
	TArray<URHJsonPushNotification*> Notifications;

	UPROPERTY(Transient)
	ERHPushNotificationState State;

	UPROPERTY(Transient)
	bool bNotificationPermissionGranted;

private:
	void OnCheckShouldShowResponse(bool bShouldShow, URHJsonPushNotification* Notification, const TSharedPtr<FJsonObject>* ContentObj);
};
