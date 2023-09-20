#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"

#include "DataFactories/RHQueueDataFactory.h"
#include "RHMatchInvitationModal.generated.h"

UCLASS(BlueprintType)
class RALLYHERESTART_API URHMatchInvitationModal : public URHWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void UninitializeWidget_Implementation() override;

protected: // BP Interface
	// Accept the invitation, optionally voting on a map (currently unsupported, but in for future use)
	UFUNCTION(BlueprintCallable, Category="Match Invitation|Action")
	void AcceptInvite(int32 MapId = 0);

	// Declines the current invitation
	UFUNCTION(BlueprintCallable, Category="Match Invitation|Action")
	void DeclineInvite();

	// For use with PopupManager, calls AcceptInvite(0)
	UFUNCTION(BlueprintCallable, Category = "Match Invitation|Action")
	void AcceptInviteDefault();

	// Gives us how much time, in seconds, is left until this invite expires
	UFUNCTION(BlueprintPure, Category="Match Invitation")
	float GetInvitationTimeRemaining() const;

	// Gives us how much time, in seconds, we had total when we first received this invite
	UFUNCTION(BlueprintPure, Category="Match Invitation")
	float GetInvitationTotalTimeToExpire() const;

	UFUNCTION(BlueprintCallable, Category="Match Invitation|Action")
	void CloseScreen();

	UFUNCTION(BlueprintPure, Category="Match Invitation")
	class URHQueueDataFactory* GetQueueDataFactory() const;

	UFUNCTION(BlueprintPure, Category = "Match Invitation")
	URH_FriendSubsystem* GetRH_LocalPlayerFriendSubsystem() const;

private:
	UFUNCTION()
	void OnInvitationExpired();

	void ClearCurrentInvite();

private:
	FGuid PendingInvitation;

	UPROPERTY()
	FTimerHandle InvitationExpireTimeout;

	int32 PopupId;
	FText LastInviterName;
};