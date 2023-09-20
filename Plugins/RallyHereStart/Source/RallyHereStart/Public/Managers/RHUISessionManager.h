#pragma once
#include "Online/CoreOnline.h"
#include "RallyHereIntegrationModule.h"
#include "RH_CatalogSubsystem.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RHUISessionManager.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPlatformActivitiesReceived)
DECLARE_MULTICAST_DELEGATE(FOnPlatformActivityDescriptionsReceived)
DECLARE_MULTICAST_DELEGATE(FOnPlatformActivitiesReady)
DECLARE_MULTICAST_DELEGATE(FOnPlayerInventoryReady)

class URHGameInstance;

UCLASS()
class RALLYHERESTART_API URHUISessionData : public UObject
{
	GENERATED_BODY()

public:
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
};

UCLASS(Config = Game)
class RALLYHERESTART_API URHUISessionManager : public UObject
{
    GENERATED_BODY()

public:
    // Initialize the UI Session Manager
    void Initialize(URHGameInstance* InGameInstance);

    // Clean Up the UI Session Manager
    void Uninitialize();

	void OnLoginPlayerChanged(ULocalPlayer* LocalPlayer);

	bool HasFullInventory(URH_PlayerInfo* PlayerInfo) const
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			return SessionData->bHasFullPlayerInventory;
		}
		return false;
	}

	// Returns if the News Panel should be shown
    bool ShouldShowNewsPanel(URH_PlayerInfo* PlayerInfo) const
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			return SessionData->bNeedsToShowNewsPanel;
		}
		return false;
	}

    // Returns if the News Panel should be shown
    void SetNewsPanelSeen(URH_PlayerInfo* PlayerInfo)
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			SessionData->bNeedsToShowNewsPanel = false;
		}
	}

    // Returns if the Voucher Redemption Status needs to be checked
    bool ShouldCheckForVouchers(URH_PlayerInfo* PlayerInfo) const
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			return SessionData->bNeedsToRedeemVoucher;
		}
		return false;
	}

    // Sets the Voucher Redemption Has been checked
    void SetHasCheckedForVouchers(URH_PlayerInfo* PlayerInfo)
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			SessionData->bNeedsToRedeemVoucher = false;
		}
	}

	// Replies from platforms that we have gotten their Achievements
    void OnQueryAchievementsComplete(const FUniqueNetId& UserId, const bool bSuccess, URH_PlayerInfo* PlayerInfo);
    void OnQueryAchievementDescriptionsComplete(const FUniqueNetId& UserId, const bool bSuccess, URH_PlayerInfo* PlayerInfo);

    // Checks if Platform Achievements are ready
	bool HasPlatformAchievements(URH_PlayerInfo* PlayerInfo, bool RequireDescriptions = true) const
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			return SessionData->bHasPlatformAchievements && (!RequireDescriptions || SessionData->bHasPlatformAchievementDescriptions);
		}

		return false;
	}

    // Gets if the current platform has Achievement Integration
	bool HasAchievementIntegration(URH_PlayerInfo* PlayerInfo) const
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			return SessionData->bAchievementIntegrationEnabled;
		}
		return false;
	}

	// Adds News Panel Unique Id to the array
	void AddNewsPanelId(FName UniqueId, URH_PlayerInfo* PlayerInfo)
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			if (UniqueId != NAME_None)
			{
				SessionData->ShownNewsPanelIds.Add(UniqueId);
			}
		}
	}

	// Gets the array of News Panel Unique Id
	TArray<FName> GetNewsPanelIds(URH_PlayerInfo* PlayerInfo) const 
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			return SessionData->ShownNewsPanelIds;
		}
		return TArray<FName>(); 
	}

	FDateTime GetPlayerLoggedInTime(URH_PlayerInfo* PlayerInfo) const
	{
		if (URHUISessionData* SessionData = SessionDataPerPlayer.FindRef(PlayerInfo))
		{
			return SessionData->LoggedInTime;
		}
		return FDateTime::MinValue();
	}

	void ProcessRedemptionRewards(URH_PlayerInfo* PlayerInfo);

	UPROPERTY(Transient)
	TMap<URH_PlayerInfo*, URHUISessionData*> SessionDataPerPlayer;

protected:

	UPROPERTY(Config)
	int32 RewardRedemptionVendorId;

	// Evaluated a loot table to see how many times the player is able to claim it
	void GetClaimableQuantity(FRHAPI_Loot LootItem, URH_CatalogSubsystem* CatalogSubsystem, URH_PlayerInfo* PlayerInfo, int32 LootIndex, int32 Quantity, const FRH_GetInventoryCountBlock& Delegate = FRH_GetInventoryCountBlock());

	UPROPERTY(Transient)
	URHGameInstance* GameInstance;
};