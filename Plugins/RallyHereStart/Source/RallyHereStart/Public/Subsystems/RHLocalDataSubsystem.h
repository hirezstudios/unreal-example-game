#pragma once
#include "Online/CoreOnline.h"
#include "RallyHereIntegrationModule.h"
#include "RH_CatalogSubsystem.h"
#include "RH_PlayerInfoSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "RHLocalDataSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPlatformActivitiesReceived)
DECLARE_MULTICAST_DELEGATE(FOnPlatformActivityDescriptionsReceived)
DECLARE_MULTICAST_DELEGATE(FOnPlatformActivitiesReady)
DECLARE_MULTICAST_DELEGATE(FOnPlayerInventoryReady)

UCLASS(Config = Game)
class RALLYHERESTART_API URHLocalDataSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OnLoginPlayerChanged();

	bool HasFullInventory() const
	{
		return bHasFullPlayerInventory;
	}

	// Returns if the News Panel should be shown
    bool ShouldShowNewsPanel() const
	{
		return bNeedsToShowNewsPanel;
	}

    // Returns if the News Panel should be shown
    void SetNewsPanelSeen()
	{
		bNeedsToShowNewsPanel = false;
	}

    // Returns if the Voucher Redemption Status needs to be checked
    bool ShouldCheckForVouchers() const
	{
		return bNeedsToRedeemVoucher;
	}

    // Sets the Voucher Redemption Has been checked
    void SetHasCheckedForVouchers()
	{
		bNeedsToRedeemVoucher = false;
	}

	// Replies from platforms that we have gotten their Achievements
    void OnQueryAchievementsComplete(const FUniqueNetId& UserId, const bool bSuccess);
    void OnQueryAchievementDescriptionsComplete(const FUniqueNetId& UserId, const bool bSuccess);

    // Checks if Platform Achievements are ready
	bool HasPlatformAchievements(bool RequireDescriptions = true) const
	{
		return bHasPlatformAchievements && (!RequireDescriptions || bHasPlatformAchievementDescriptions);
	}

    // Gets if the current platform has Achievement Integration
	bool HasAchievementIntegration() const
	{
		return bAchievementIntegrationEnabled;
	}

	// Adds News Panel Unique Id to the array
	void AddNewsPanelId(FName UniqueId)
	{
		if (UniqueId != NAME_None)
		{
			ShownNewsPanelIds.Add(UniqueId);
		}
	}

	// Gets the array of News Panel Unique Id
	TArray<FName> GetNewsPanelIds() const
	{
		return ShownNewsPanelIds;
	}

	FDateTime GetPlayerLoggedInTime() const
	{
		return LoggedInTime;
	}

	void ProcessRedemptionRewards();

	FOnPlayerInventoryReady OnPlayerInventoryReady;

	// Delegates for various platform achievement state updates
	FOnPlatformActivitiesReceived OnPlatformActivitiesReceived;
	FOnPlatformActivityDescriptionsReceived OnPlatformActivityDescriptionsReceived;
	FOnPlatformActivitiesReady OnPlatformActivitiesReady;

	// Resets the state of the session to a logged off player state
	void Clear()
	{
		bNeedsToShowNewsPanel = true;
		bNeedsToRedeemVoucher = true;
		bHasPlatformAchievements = false;
		bHasPlatformAchievementDescriptions = false;
		bAchievementIntegrationEnabled = false;
		bHasFullPlayerInventory = false;
		ShownNewsPanelIds.Empty();
		LoggedInTime = FDateTime::MinValue();
	}

protected:

	// Stores if the player needs to be presented the News Panel
	UPROPERTY(Transient)
	bool bNeedsToShowNewsPanel;

	// Stores the unique id of the news panel that shown
	UPROPERTY(Transient)
	TArray<FName> ShownNewsPanelIds;

	// Stores if the player needs their voucher redemption status checked
	UPROPERTY(Transient)
	bool bNeedsToRedeemVoucher;

	// Stores if the platform achievements have been retrieved for the given player
	UPROPERTY(Transient)
	bool bHasPlatformAchievements;

	// Stores if the platform achievement descriptions have been retrieved for the given player
	UPROPERTY(Transient)
	bool bHasPlatformAchievementDescriptions;

	// Stores if the current platform has achievement integration enabled
	UPROPERTY(Transient)
	bool bAchievementIntegrationEnabled;

	UPROPERTY(Transient)
	bool bHasFullPlayerInventory;

	// Timestamp of first login
	UPROPERTY(Transient)
	FDateTime LoggedInTime;

	UPROPERTY(Config)
	int32 RewardRedemptionVendorId;

	// Evaluated a loot table to see how many times the player is able to claim it
	void GetClaimableQuantity(FRHAPI_Loot LootItem, URH_CatalogSubsystem* CatalogSubsystem, URH_PlayerInfo* PlayerInfo, int32 LootIndex, int32 Quantity, const FRH_GetInventoryCountBlock& Delegate = FRH_GetInventoryCountBlock());
};