// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Shared/Widgets/RHWidget.h"
#include "Managers/RHPartyManager.h"
#include "RHToastNotificationWidgetBase.generated.h"

class URHStoreItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToastReceived);

UENUM(BlueprintType)
enum class EToastCategory : uint8
{
    ETOAST_INFO                 UMETA(DisplayName = "Info"),
    ETOAST_ERROR                UMETA(DisplayName = "Error"),
    ETOAST_FRIEND               UMETA(DisplayName = "Friend"),
    ETOAST_PARTY                UMETA(DisplayName = "Party"),
    ETOAST_CHALLENGE            UMETA(DisplayName = "Challenge"),
    ETOAST_MERC_MASTERY         UMETA(DisplayName = "MercMastery"),
    ETOAST_ITEM_UNLOCK          UMETA(DisplayName = "ItemUnlock"),
    ETOAST_AWARD                UMETA(DisplayName = "Award"),
    ETOAST_BATTLEPASS_TIER      UMETA(DisplayName = "BattlePassTier"),
	ETOAST_BOOST_ACTIVATION	    UMETA(DisplayName = "BoostActivation"),
	ETOAST_PLAYER_LEVEL		    UMETA(DisplayName = "PlayerLevel"),
    ETOAST_EVENT_CHALLENGE      UMETA(DisplayName = "Event Challenge"),
	ETOAST_WEAPON_MASTERY		UMETA(DisplayName = "Weapon Mastery")
};

USTRUCT(BlueprintType)
struct FToastData
{
    GENERATED_USTRUCT_BODY()
public:
    FToastData()
        : ToastCategory(EToastCategory::ETOAST_INFO)
        , Reward(nullptr)
        , OptionalItemValue(nullptr)
        , OptionalIntValue(0)
    {}

    // Type of toast
    UPROPERTY(BlueprintReadOnly)
    EToastCategory              ToastCategory;

    // Title to be displayed on the toast, if the type displays a title
    UPROPERTY(BlueprintReadOnly)
    FText                       Title;

    // Message displayed on the toast, most toasts use this
    UPROPERTY(BlueprintReadOnly)
    FText                       Message;

    // Reward item associated with the toast
    UPROPERTY(BlueprintReadOnly)
    URHStoreItem*            Reward;

    // Additional Item information for a toast
    UPROPERTY(BlueprintReadOnly)
    UPlatformInventoryItem*     OptionalItemValue;

    // Additional information for a toast
    UPROPERTY(BlueprintReadOnly)
    int32                       OptionalIntValue;
};

UCLASS()
class RALLYHERESTART_API URHToastNotificationWidgetBase : public URHWidget
{
    GENERATED_BODY()
public:

    URHToastNotificationWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void InitializeWidget_Implementation() override;

    UFUNCTION(BlueprintImplementableEvent)
    void OnToastNotificationReceived(FToastData ToastData);

    UPROPERTY(BlueprintAssignable, Category = "Toast Notification")
    FOnToastReceived OnToastReceived;

protected:
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Toast Notification")
    int32 MaxToastNotification;
    UPROPERTY(BlueprintReadOnly, Category = "Toast Notification")
    int32 CurrentToastCount;

    UFUNCTION(BlueprintPure)
    URH_FriendSubsystem* GetFriendSubsystem();

    UFUNCTION(BlueprintPure)
    URHPartyManager* GetPartyManager();

	// Internally increments the toast count
	UFUNCTION(BlueprintCallable, Category = "Toast Notification")
	void NotifyToastShown();

	// Internally decrements the toast count and attempts to queue more if needed
	UFUNCTION(BlueprintCallable, Category = "Toast Notification")
	void NotifyToastHidden();

    UFUNCTION()
    void OnFriendUpdated(URH_RHFriendAndPlatformFriend* UpdatedFriend);

    UFUNCTION()
    void HandlePartyInviteReceived(URH_PlayerInfo* PartyInviter);
    UFUNCTION()
    void HandlePartyInviteError(FText PlayerName);
    UFUNCTION()
    void HandlePartyLocalPlayerLeft();
    UFUNCTION()
    void HandlePartyMemberAdded(FRH_PartyMemberData PartyMemberData);
    UFUNCTION()
    void HandlePartyInviteAccepted();
    UFUNCTION()
    void HandlePartyInviteRejected();
    UFUNCTION()
    void HandlePartyMemberKick(const FGuid& PlayerId);
    UFUNCTION()
    void HandlePartyMemberLeftGeneric();
	UFUNCTION()
	void HandlePartyMemberLeft(FRH_PartyMemberData PartyMemberData);
    UFUNCTION()
    void HandlePartyInviteSent(URH_PlayerInfo* Invitee);
    UFUNCTION()
    void HandlePartyMemberPromoted(const FGuid& PlayerId);
	UFUNCTION()
	void HandlePartyDisbanded();

	FGuid GetLocalPlayerUuid() const;

    UFUNCTION(BlueprintCallable, Category = "Toast Notification")
    void ShowToastNotification();

    UFUNCTION(BlueprintCallable, Category = "Toast Notification")
    void StoreToastQueue(FToastData ToastNotification);

    UFUNCTION(BlueprintCallable, Category = "Toast Notification")
    bool GetNext(FToastData& NextToastNotification);

    UFUNCTION(BlueprintCallable, Category = "Toast Notification")
    void ClearNotificationQueue();

	UFUNCTION(BlueprintCallable, Category = "Toast Notification")
	void ClearPostMatchQueue();

    UPROPERTY(BlueprintReadWrite, Category = "Toast Notification")
    bool IsBusy;

	UFUNCTION(BlueprintCallable, Category = "Toast Notification")
	FORCEINLINE TArray<FToastData> GetPostMatchToasts() const { return PostMatchToasts; }

private:

    UPROPERTY()
    TArray<FToastData> ToastQueue;

	UPROPERTY()
	TArray<FToastData> PostMatchToasts;

	void HandleGetDisplayNameResponse(bool bSuccess, const FString& DisplayName, URH_PlayerInfo* PartyInviter, EToastCategory ToastCategory, FText Message);
};