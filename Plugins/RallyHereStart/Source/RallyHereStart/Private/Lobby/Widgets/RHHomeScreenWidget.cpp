// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "GameFramework/RHGameInstance.h"
#include "GameFramework/RHGameUserSettings.h"
#include "Subsystems/RHOrderSubsystem.h"
#include "Subsystems/RHStoreSubsystem.h"
#include "Subsystems/RHLocalDataSubsystem.h"
#include "Subsystems/RHNewsSubsystem.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Lobby/Widgets/RHWhatsNewModal.h"
#include "Lobby/Widgets/RHHomeScreenWidget.h"

void URHHomeScreenWidget::CheckForOnShownEvents()
{
	if (CheckForVoucherRedemption())
	{
		return;
	}

	if (CheckForWhatsNewModal())
	{
		return;
	}
}

bool URHHomeScreenWidget::CheckForVoucherRedemption()
{
	if (URHOrderSubsystem* OrderSubsystem = MyHud->GetOrderSubsystem())
	{
		if (URHLocalDataSubsystem* LocalDataSubsystem = MyHud->GetLocalDataSubsystem())
		{
			if (LocalDataSubsystem->ShouldCheckForVouchers())
			{
				if (URHStoreSubsystem* StoreSubsystem = GetGameInstance()->GetSubsystem<URHStoreSubsystem>())
				{
					TArray<URHStoreItem*> StoreItems;
					TArray<URHStoreItem*> VoucherItems;

					LocalDataSubsystem->SetHasCheckedForVouchers();

					StoreSubsystem->GetAffordableVoucherItems(MyHud->GetLocalPlayerInfo(), FRH_GetAffordableItemsInVendorDelegate::CreateWeakLambda(this, [this, OrderSubsystem](TArray<URHStoreItem*> StoreItems, TArray<URHStoreItem*> VoucherItems)
						{
							if (VoucherItems.Num() > 0)
							{
								for (int32 i = 0; i < VoucherItems.Num(); ++i)
								{
									OrderSubsystem->CreateOrderForItem(VoucherItems[i], MyHud->GetLocalPlayerInfo());
								}
							}
							else
							{
								CheckForOnShownEvents();
							}
						}));

					return true;
				}
			}
		}
	}

	return false;
}

bool URHHomeScreenWidget::CheckForWhatsNewModal()
{
	TArray<URHJsonData*> PanelsToCheck;

	if (ARHLobbyHUD* LobbyHud = Cast<ARHLobbyHUD>(MyHud))
	{
		if (URHLocalDataSubsystem* LocalDataSubsystem = LobbyHud->GetLocalDataSubsystem())
		{
			if (LocalDataSubsystem->ShouldShowNewsPanel())
			{
				if (URHNewsSubsystem* pNewsSubsystem = GetGameInstance()->GetSubsystem<URHNewsSubsystem>())
				{
					TSharedPtr<FJsonObject> LandingPanelJson = pNewsSubsystem->GetJsonPanelByName(TEXT("landingpanel"));

					if (LandingPanelJson.IsValid())
					{
						const TSharedPtr<FJsonObject>* WhatsNewObj;

						if (LandingPanelJson.Get()->TryGetObjectField(TEXT("whatsNewPanel"), WhatsNewObj))
						{
							// Check panel data, only proceed if there is a valid imageUrl
							const TArray<TSharedPtr<FJsonValue>>* WhatsNewPanelContents;

							if ((*WhatsNewObj)->TryGetArrayField(TEXT("content"), WhatsNewPanelContents))
							{
								TArray<TSharedPtr<FJsonValue>> SortedPanelContents = *WhatsNewPanelContents;

								for (TSharedPtr<FJsonValue> WhatsNewPanels : SortedPanelContents)
								{
									const TSharedPtr<FJsonObject>* WhatsNewPanelObj;

									if (WhatsNewPanels->TryGetObject(WhatsNewPanelObj))
									{
										if (URHWhatsNewPanel* Panel = NewObject<URHWhatsNewPanel>())
										{
											// Only show if panel should show on this platform
											pNewsSubsystem->LoadData(Panel, WhatsNewPanelObj);
											const TSharedPtr<FJsonObject>* ObjectField;
											FString PanelIdString = "";

											if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("imageUrl"), ObjectField))
											{
												Panel->Image = pNewsSubsystem->GetTextureByRemoteURL(ObjectField);

												// Only show if there is a valid image
												if (Panel->Image)
												{
													int32 VersionNumber;
													bool ShowOnlyIfNew;

													if ((*WhatsNewObj)->TryGetBoolField(TEXT("showOnlyIfNew"), ShowOnlyIfNew))
													{
														if (ShowOnlyIfNew)
														{
															// Check if we have a new version
															if ((*WhatsNewObj)->TryGetNumberField(TEXT("version"), VersionNumber))
															{
																if (URHGameUserSettings* const pRHGameSettings = Cast<URHGameUserSettings>(GEngine->GetGameUserSettings()))
																{
																	if (VersionNumber > pRHGameSettings->GetWhatsNewVersion())
																	{
																		pRHGameSettings->SaveWhatsNewVersion(VersionNumber);
																		pRHGameSettings->SaveSettings();
																		PanelsToCheck.Add(Panel);
																	}
																}
															}
														}
														else
														{
															PanelsToCheck.Add(Panel);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}

					// Clear the session manager for this login of the news seen
					LocalDataSubsystem->SetNewsPanelSeen();
					pNewsSubsystem->CheckShouldShowForPlayer(PanelsToCheck, LobbyHud->GetLocalPlayerInfo(), FOnGetShouldShowPanels::CreateLambda([this](FRHShouldShowPanelsWrapper Wrapper)
						{
							for (const auto& pair : Wrapper.ShouldShowByPanel)
							{
								if (pair.Value)
								{
									AddViewRoute(NewsViewTag);  //$$ KAB - Route names changed to Gameplay Tags
									return;
								}
							}
						
							CheckForOnShownEvents();
						}));

					return true;
				}
			}
		}
	}

	return false;
}
