#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/RHGameInstance.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Subsystems/RHOrderSubsystem.h"
#include "RH_CatalogSubsystem.h"
#include "RH_LocalPlayerSubsystem.h"
#include "RH_PlayerInfoSubsystem.h"
#include "RH_GameInstanceSubsystem.h"
#include "RH_MatchmakingBrowser.h"
#include "RH_EntitlementSubsystem.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Subsystems/RHLocalDataSubsystem.h"

bool URHLocalDataSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Local Players should always have this subsystem?
	return true;
}

void URHLocalDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (URH_LocalPlayerSubsystem* RHSS = LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>())
		{
			if (RHSS->GetAuthContext().IsValid())
			{
				RHSS->GetAuthContext()->OnLoginUserChanged().AddUObject(this, &URHLocalDataSubsystem::OnLoginPlayerChanged);
			}
		}
	}
}

void URHLocalDataSubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (URH_LocalPlayerSubsystem* RHSS = LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>())
		{
			if (RHSS->GetAuthContext().IsValid())
			{
				RHSS->GetAuthContext()->OnLoginUserChanged().RemoveAll(this);
			}
		}
	}
}

void URHLocalDataSubsystem::OnLoginPlayerChanged()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	URH_LocalPlayerSubsystem* RHSS = LocalPlayer ? LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>() : nullptr;
	URH_PlayerInfo* PlayerInfo = nullptr;
	URHGameInstance* GameInstance = LocalPlayer ? Cast<URHGameInstance>(LocalPlayer->GetGameInstance()) : nullptr;

	if (RHSS != nullptr && GameInstance != nullptr)
	{
		PlayerInfo = RHSS->GetLocalPlayerInfo();
		
		if (PlayerInfo != nullptr)
		{
			if (URH_PlayerInventory* PlayerInventory = PlayerInfo->GetPlayerInventory())
			{
				PlayerInventory->GetInventory(TArray<int32>(), FRH_OnInventoryUpdateDelegate::CreateWeakLambda(this, [this, GameInstance](bool bSuccess)
					{
						if (bSuccess)
						{
							bHasFullPlayerInventory = true;
							OnPlayerInventoryReady.Broadcast();
							
							if (RewardRedemptionVendorId > 0)
							{
								if (URHStoreSubsystem* StoreSubsystem = GameInstance->GetSubsystem<URHStoreSubsystem>())
								{
									TArray<int32> VendorIds;
									VendorIds.Push(RewardRedemptionVendorId);

									if (VendorIds.Num())
									{
										StoreSubsystem->RequestVendorData(VendorIds, FRH_CatalogCallDelegate::CreateLambda([this](bool bSuccess)
											{
												ProcessRedemptionRewards();
											}));
									}
								}
							}
						}
					}));
			}
		}

		if (URH_EntitlementSubsystem* ESS = RHSS->GetEntitlementSubsystem())
		{
			ESS->SubmitEntitlementsForOSS(ERHAPI_Platform::Amazon);
			ESS->SubmitEntitlementsForOSS(ERHAPI_Platform::Twitch);
		}

		if (PlayerInfo != nullptr)
		{
			PlayerInfo->GetLinkedPlatformInfo(FTimespan(), false, FRH_PlayerInfoGetPlatformsDelegate::CreateWeakLambda(this, [this, RHSS](bool bSuccess, const TArray<URH_PlayerPlatformInfo*>& Platforms)
				{
					for (const auto& Platform : Platforms)
					{
						if (Platform != nullptr && Platform->GetPlatform() == RHSS->GetLoggedInPlatform())
						{
							const auto IdentityInterface = Online::GetIdentityInterface();
							TSharedPtr<const FUniqueNetId> PlayerUniqueNetId = IdentityInterface->CreateUniquePlayerId(Platform->GetPlatformUserId());

							if (PlayerUniqueNetId.IsValid())
							{
								if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
								{
									IOnlineAchievementsPtr Achievements = OSS->GetAchievementsInterface();
									if (Achievements.IsValid())
									{
									//	FOnQueryAchievementsCompleteDelegate QueryAchievementsDel;
									//	FOnQueryAchievementsCompleteDelegate QueryAchievementDescriptionsDel;

									//	QueryAchievementsDel.BindUObject(this, &URHLocalDataSubsystem::OnQueryAchievementsComplete);
									//	QueryAchievementDescriptionsDel.BindUObject(this, &URHLocalDataSubsystem::OnQueryAchievementDescriptionsComplete);

									//	Achievements->QueryAchievements(*PlayerUniqueNetId.Get(), QueryAchievementsDel);
									//	Achievements->QueryAchievementDescriptions(*PlayerUniqueNetId.Get(), QueryAchievementDescriptionsDel);
									}
								}
							}
						}
					}
				}));
		}
	}

	Clear();
	LoggedInTime = FDateTime::UtcNow(); // #RHTODO - PLAT-4591 - We need to actual server timestamp not players local time
	
	if (GameInstance)
	{
		// Fetch region settings on login
		if (URH_GameInstanceSubsystem* pGISS = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
		{
			if (auto* pMMCache = pGISS->GetMatchmakingCache())
			{
				pMMCache->SearchRegions();
			}
		}
	}
}

void URHLocalDataSubsystem::OnQueryAchievementsComplete(const FUniqueNetId& UserId, const bool bSuccess)
{
	bHasPlatformAchievements = bSuccess;
	OnPlatformActivitiesReceived.Broadcast();

	if (HasPlatformAchievements())
	{
		OnPlatformActivitiesReady.Broadcast();
	}
}

void URHLocalDataSubsystem::OnQueryAchievementDescriptionsComplete(const FUniqueNetId& UserId, const bool bSuccess)
{
	bHasPlatformAchievementDescriptions = bSuccess;
	OnPlatformActivityDescriptionsReceived.Broadcast();

	if (HasPlatformAchievements())
	{
		OnPlatformActivitiesReady.Broadcast();
	}
}

void URHLocalDataSubsystem::ProcessRedemptionRewards()
{
	// Traverse the redemption vendor to check for payouts from it
	URH_CatalogSubsystem* CatalogSubsystem = nullptr;
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	URHGameInstance* GameInstance = LocalPlayer ? Cast<URHGameInstance>(LocalPlayer->GetGameInstance()) : nullptr;

	if (GameInstance)
	{
		if (auto pGISubsystem = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
		{
			CatalogSubsystem = pGISubsystem->GetCatalogSubsystem();
		}
	}

	URH_LocalPlayerSubsystem* RHSS = LocalPlayer ? LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>() : nullptr;
	URH_PlayerInfo* PlayerInfo = RHSS != nullptr ? RHSS->GetLocalPlayerInfo() : nullptr;

	if (CatalogSubsystem != nullptr && PlayerInfo != nullptr)
	{		
		FRHAPI_Vendor Vendor;
		if (CatalogSubsystem->GetVendorById(RewardRedemptionVendorId, Vendor))
		{
			if (const auto& LootItems = Vendor.GetLootOrNull())
			{
				for (const auto& VendorItemPair : (*LootItems))
				{
					const FRHAPI_Loot& LootItem = VendorItemPair.Value;
					
					if (LootItem.GetActive(false) && LootItem.GetIsClaimableByClient(false) && LootItem.GetSubVendorId(0) != 0)
					{
						GetClaimableQuantity(LootItem, CatalogSubsystem, PlayerInfo, 0, 0, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [this, LootItem, PlayerInfo, GameInstance](int32 QuantityToRedeem)
							{
								if (QuantityToRedeem > 0)
								{
									TArray<URH_PlayerOrderEntry*> PlayerOrderEntries;

									URH_PlayerOrderEntry* NewPlayerOrderEntry = NewObject<URH_PlayerOrderEntry>();
									NewPlayerOrderEntry->FillType = ERHAPI_PlayerOrderEntryType::PurchaseLoot;
									NewPlayerOrderEntry->LootId = LootItem.GetLootId();
									NewPlayerOrderEntry->Quantity = QuantityToRedeem;
									NewPlayerOrderEntry->ExternalTransactionId = "Reward Redemption During Login";
									PlayerOrderEntries.Push(NewPlayerOrderEntry);

									PlayerInfo->GetPlayerInventory()->CreateNewPlayerOrder(ERHAPI_Source::Client, false, PlayerOrderEntries, FRH_OrderResultDelegate::CreateLambda([this, GameInstance](const URH_PlayerInfo* PlayerInfo, TArray<URH_PlayerOrderEntry*> OrderEntries, const FRHAPI_PlayerOrder& PlayerOrder)
										{
											if (URHOrderSubsystem* OrderSubsystem = GameInstance->GetSubsystem<URHOrderSubsystem>())
											{
												TArray<FRHAPI_PlayerOrder> OrderResults;
												OrderResults.Push(PlayerOrder);
												OrderSubsystem->OnPlayerOrder(OrderResults, PlayerInfo);
											}
										}));
								}
							}));
					}
				}
			}
		}
	}
}

void URHLocalDataSubsystem::GetClaimableQuantity(FRHAPI_Loot LootItem, URH_CatalogSubsystem* CatalogSubsystem, URH_PlayerInfo* PlayerInfo, int32 LootIndex, int32 Quantity, const FRH_GetInventoryCountBlock& Delegate)
{
	int32 QuantityToRedeem = Quantity;

	if (CatalogSubsystem != nullptr)
	{
		FRHAPI_Vendor Vendor;
		if (CatalogSubsystem->GetVendorById(LootItem.GetSubVendorId(0), Vendor))
		{
			TArray<FRHAPI_Loot> RedemptionVendorRecipeItems;

			if (const auto& LootItems = Vendor.GetLootOrNull())
			{
				for (const auto& VendorItemPair : (*LootItems))
				{
					// Build out and sort the items from the given vendor so we can evaluate the recipe
					const FRHAPI_Loot& RedemptionItem = VendorItemPair.Value;

					if (RedemptionItem.GetActive(false))
					{
						RedemptionVendorRecipeItems.Add(RedemptionItem);
					}
				}
			}

			RedemptionVendorRecipeItems.Sort([](const FRHAPI_Loot& A, const FRHAPI_Loot& B) { return (A.GetSortOrder(0) < B.GetSortOrder(0)); });

			for (int32 i = LootIndex; i < RedemptionVendorRecipeItems.Num(); ++i)
			{
				const FRHAPI_Loot& RecipeItem = RedemptionVendorRecipeItems[i];

				int32 InternalRedeemCount = 0;

				if (RecipeItem.GetSubVendorId(0) != 0)
				{
					// Create a sub thread for this recipe item, then when it completes continue where we left off.
					GetClaimableQuantity(RecipeItem, CatalogSubsystem, PlayerInfo, 0, 0, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [this, LootItem, PlayerInfo, CatalogSubsystem, QuantityToRedeem, i, Delegate](int32 InternalRedeemCount)
						{
							// If an internal bundle has more redeem counts than we have externally redeem this multiple times
							int32 NewQty = QuantityToRedeem;
							if (InternalRedeemCount > QuantityToRedeem)
							{
								NewQty = InternalRedeemCount;
							}

							GetClaimableQuantity(LootItem, CatalogSubsystem, PlayerInfo, i + 1, NewQty, Delegate);
						}));
					return;
				}
				else if (RecipeItem.GetItemId(0) != 0)
				{
					if (URH_PlayerInventory* PlayerInventory = PlayerInfo->GetPlayerInventory())
					{
						PlayerInventory->GetInventoryCount(RecipeItem.GetItemId(0), FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [this, RecipeItem, LootItem, PlayerInfo, CatalogSubsystem, QuantityToRedeem, i, Delegate](int32 InventoryCount)
							{
								int32 NewQty = QuantityToRedeem;

								switch (RecipeItem.GetInventoryOperation(ERHAPI_InventoryOperation::Invalid))
								{
									case ERHAPI_InventoryOperation::CheckLessThan:
										if (InventoryCount >= RecipeItem.GetQuantity())
										{
											// Early out, redeem whatever amount we have calculated up to this point
											Delegate.ExecuteIfBound(QuantityToRedeem);
											return;
										}
										break;
									case ERHAPI_InventoryOperation::CheckGreaterThanOrEqualAndSubtract:
										// If we are going to start consuming something of yours, count how many times we can consume it
										if (InventoryCount > 0)
										{
											int32 InternalRedeemCount = InventoryCount / RecipeItem.GetQuantity();

											if (InternalRedeemCount > QuantityToRedeem)
											{
												NewQty = InternalRedeemCount;
											}
										}
										// FALLTHOUGH
									case ERHAPI_InventoryOperation::CheckGreaterThanOrEqual:
										if (InventoryCount < RecipeItem.GetQuantity())
										{
											// Early out, redeem whatever amount we have calculated up to this point
											Delegate.ExecuteIfBound(QuantityToRedeem);
											return;
										}
										break;
									case ERHAPI_InventoryOperation::Subtract:
									case ERHAPI_InventoryOperation::Set:
									case ERHAPI_InventoryOperation::Add:
										// If we make it to this level of item type then we have are actually getting something, claim this LTI
										Delegate.ExecuteIfBound(QuantityToRedeem > 0 ? QuantityToRedeem : 1);
										return;
										break;
									case ERHAPI_InventoryOperation::Invalid:
									default:
										// Do nothing in these cases with evaluating the item
										break;
								}
								GetClaimableQuantity(LootItem, CatalogSubsystem, PlayerInfo, i + 1, NewQty, Delegate);
							}));
						return;
					}
				}
			}
		}
	}
	Delegate.ExecuteIfBound(QuantityToRedeem);
}