#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/RHNewsSubsystem.h"
#include "RHPushNotificationSubsystem.generated.h"

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
class RALLYHERESTART_API URHPushNotificationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void CheckNotificationState();

	void SetState(ERHPushNotificationState NewState);

	void ScheduleNotifications();

	UFUNCTION()
	void HandleRegisteredForUserNotifications(int32 Types);

	UFUNCTION()
	void HandleRegisteredForRemoteNotifications(TArray<uint8> InToken);

	UFUNCTION()
	void HandleJsonReady(const FString& JsonName);

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
