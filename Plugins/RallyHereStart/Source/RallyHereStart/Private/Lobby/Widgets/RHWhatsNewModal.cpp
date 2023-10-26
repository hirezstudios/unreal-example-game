// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Subsystems/RHNewsSubsystem.h"
#include "Subsystems/RHLocalDataSubsystem.h"
#include "Lobby/Widgets/RHWhatsNewModal.h"

URHWhatsNewModal::URHWhatsNewModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	maxPanelCount = 3;
	StoredPanels.Empty();
}

void URHWhatsNewModal::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();

    if (URHNewsSubsystem* pNewsSubsystem = GetNewsSubsystem())
    {
        pNewsSubsystem->JsonPanelUpdated.AddDynamic(this, &URHWhatsNewModal::UpdateWhatsNewPanels);
		// Simulate checking for the proper json
		UpdateWhatsNewPanels(TEXT("landingpanel"));
	}
}

void URHWhatsNewModal::UninitializeWidget_Implementation()
{
    Super::UninitializeWidget_Implementation();

    if (URHNewsSubsystem* pNewsSubsystem = GetNewsSubsystem())
    {
        pNewsSubsystem->JsonPanelUpdated.RemoveDynamic(this, &URHWhatsNewModal::UpdateWhatsNewPanels);
    }
}

void URHWhatsNewModal::UpdateWhatsNewPanels(const FString& JsonName)
{
	if (JsonName != TEXT("landingpanel") && JsonName != TEXT("itemsUpdated"))
	{
		return;
	}

	if (URHNewsSubsystem* pNewsSubsystem = GetNewsSubsystem())
	{
		TSharedPtr<FJsonObject> LandingPanelJson = pNewsSubsystem->GetJsonPanelByName(TEXT("landingpanel"));

		if (LandingPanelJson.IsValid())
		{
			const TSharedPtr<FJsonObject>* WhatsNewObj;

			if (LandingPanelJson.Get()->TryGetObjectField(TEXT("whatsNewPanel"), WhatsNewObj))
			{
				const TArray<TSharedPtr<FJsonValue>>* WhatsNewPanelContents;

				if ((*WhatsNewObj)->TryGetArrayField(TEXT("content"), WhatsNewPanelContents))
				{
					StoredPanels.Empty();
					TArray<TSharedPtr<FJsonValue>> SortedPanelContents = *WhatsNewPanelContents;
					Algo::Sort(SortedPanelContents, &RHPanel::PanelSort);

					TArray<URHWhatsNewPanel*> AllPanelContents;
					for (TSharedPtr<FJsonValue> WhatsNewPanels : SortedPanelContents)
					{
						const TSharedPtr<FJsonObject>* WhatsNewPanelObj;

						if (WhatsNewPanels->TryGetObject(WhatsNewPanelObj))
						{
							if (URHWhatsNewPanel* Panel = NewObject<URHWhatsNewPanel>())
							{
								const TSharedPtr<FJsonObject>* ObjectField;
								const TSharedPtr<FJsonObject>* ObjectField1;
								int32 NumValue = 0;

								if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("header"), ObjectField))
								{
									Panel->Header = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField));
								}
								if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("subHeader"), ObjectField))
								{
									Panel->SubHeader = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField));
								}
								if ((*WhatsNewPanelObj)->TryGetNumberField(TEXT("headerAlignment"), NumValue))
								{
									Panel->HeaderAlignment = (ENewsHeaderAlignment)NumValue;
								}

								// first sub-panel
								if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("desc1"), ObjectField))
								{
									FText Desc = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField));
									if (!Desc.IsEmpty())
									{
										if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("subHeader1"), ObjectField1))
										{
											FText Header = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField1));
											Panel->SubPanels.Add(FSubPanel(Header, Desc));
										}
										else
										{
											Panel->SubPanels.Add(FSubPanel(FText(), Desc));
										}
									}
								}

								// second sub-panel
								if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("desc2"), ObjectField))
								{
									FText Desc = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField));
									if (!Desc.IsEmpty())
									{
										if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("subHeader2"), ObjectField1))
										{
											FText Header = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField1));
											Panel->SubPanels.Add(FSubPanel(Header, Desc));
										}
										else
										{
											Panel->SubPanels.Add(FSubPanel(FText(), Desc));
										}
									}
								}

								// third sub-panel
								if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("desc3"), ObjectField))
								{
									FText Desc = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField));
									if (!Desc.IsEmpty())
									{
										if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("subHeader3"), ObjectField1))
										{
											FText Header = FText::FromString(URHNewsSubsystem::GetLocalizedStringFromObject(ObjectField1));
											Panel->SubPanels.Add(FSubPanel(Header, Desc));
										}
										else
										{
											Panel->SubPanels.Add(FSubPanel(FText(), Desc));
										}
									}
								}

								if ((*WhatsNewPanelObj)->TryGetNumberField(TEXT("alignment"), NumValue))
								{
									Panel->Alignment = (ESubPanelAlignment)NumValue;
								}

								if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("imageUrl"), ObjectField))
								{
									Panel->Image = pNewsSubsystem->GetTextureByRemoteURL(ObjectField);

									// If the URL was invalid (404) we didn't save the texture, so skip adding this panel
									if (!Panel->Image)
										continue;
								}

								if (!(*WhatsNewPanelObj)->TryGetNumberField(TEXT("bgBox"), Panel->BgBoxOpacity))
								{
									Panel->BgBoxOpacity = 0;
								}

								pNewsSubsystem->LoadData(Panel, WhatsNewPanelObj);
								StoredPanels.Push(Panel);
							}
						}
					}
				}
			}
		}
	}
	OnJsonChanged();
}

void URHWhatsNewModal::GetPanelDataAsync(FOnGetWhatsNewPanelDataBlock Delegate /*= FOnGetWhatsNewPanelDataBlock()*/)
{
    TArray<URHJsonData*> PanelsToCheck;
	int32 count = 0;
	URHGameUserSettings* const pRHGameSettings = Cast<URHGameUserSettings>(GEngine->GetGameUserSettings());

	if (MyHud.IsValid())
	{
		if (ARHLobbyHUD* LobbyHud = Cast<ARHLobbyHUD>(MyHud))
		{
			if (URHLocalDataSubsystem* LocalDataSubsystem = LobbyHud->GetLocalDataSubsystem())
			{
				TArray<FName> InstanceViewedPanelIds = LocalDataSubsystem->GetNewsPanelIds();
				if (InstanceViewedPanelIds.Num() == 0)
				{
					for (URHWhatsNewPanel* WhatsNewPanel : StoredPanels)
					{
						if (pRHGameSettings)
						{
							if (!pRHGameSettings->GetSavedViewedNewsPanelIds().Contains(WhatsNewPanel->UniqueId))
							{
								if (count < maxPanelCount)
								{
									PanelsToCheck.Add(WhatsNewPanel);
									pRHGameSettings->SaveViewedNewPanelId(WhatsNewPanel->UniqueId);
									pRHGameSettings->SaveSettings();
									LocalDataSubsystem->AddNewsPanelId(WhatsNewPanel->UniqueId);
									count++;
								}
							}
						}
					}
				}
				else
				{
					for(FName ViewedPanelId : InstanceViewedPanelIds)
					{
						for(URHWhatsNewPanel* Panel : StoredPanels)
						{
							if(Panel->UniqueId == ViewedPanelId)
							{
								PanelsToCheck.Add(Panel);
							}
						}
					}
				}
			}
		}
	}

	if (URHNewsSubsystem* pNewsSubsystem = GetNewsSubsystem())
	{
		pNewsSubsystem->CheckShouldShowForPlayer(PanelsToCheck, MyHud->GetLocalPlayerInfo(), FOnGetShouldShowPanels::CreateLambda([this, Delegate, count](FRHShouldShowPanelsWrapper Wrapper)
			{
				TArray<URHWhatsNewPanel*> PanelData;
				
				for (const auto& pair : Wrapper.ShouldShowByPanel)
				{
					if (pair.Value)
					{
						PanelData.Add(Cast<URHWhatsNewPanel>(pair.Key));
					}
				}

				// Show the last viewed panels if all the panels have been shown
				if (PanelData.Num() == 0)
				{
					int32 PanelLength = StoredPanels.Num();

					// check if should show stored panel
					if (URHNewsSubsystem* pNewsSubsystem = GetNewsSubsystem())
					{
						int32 PanelCount = 0;
						int32 CurrentCount = count;
						while (CurrentCount < maxPanelCount && PanelCount < PanelLength)
						{
							int32 index = (PanelLength - 1) - PanelCount;
							{
								PanelData.Push(StoredPanels[index]);
								CurrentCount++;
								break;
							}
							PanelCount++;
						}

						TArray<URHJsonData*> JsonPanelData(PanelData);
						pNewsSubsystem->CheckShouldShowForPlayer(JsonPanelData, MyHud->GetLocalPlayerInfo(), FOnGetShouldShowPanels::CreateLambda([this, Delegate](FRHShouldShowPanelsWrapper Wrapper)
							{
								TArray<URHWhatsNewPanel*> PanelData;

								for (const auto& pair : Wrapper.ShouldShowByPanel)
								{
									if (pair.Value)
									{
										PanelData.Add(Cast<URHWhatsNewPanel>(pair.Key));
									}
								}

								Delegate.ExecuteIfBound(PanelData);
							}));

						return;
					}
				}

				Delegate.ExecuteIfBound(PanelData);
			}));
	}
}

URHNewsSubsystem* URHWhatsNewModal::GetNewsSubsystem()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<URHNewsSubsystem>();
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHWhatsNewModal::GetNewsSubsystem Warning: GameInstance  is not currently valid."));
	}
    return nullptr;
}

bool RHPanel::PanelSort(const TSharedPtr<FJsonValue> A, const TSharedPtr<FJsonValue> B)
{
	const TSharedPtr<FJsonObject>* ObjA;
	const TSharedPtr<FJsonObject>* ObjB;
	if (A->TryGetObject(ObjA) && B->TryGetObject(ObjB))
	{
		int32 PriorityA = 0;
		int32 PriorityB = 0;

		if ((*ObjA)->TryGetNumberField(TEXT("priority"), PriorityA) && (*ObjB)->TryGetNumberField(TEXT("priority"), PriorityB))
		{
			return PriorityA > PriorityB;
		}
	}

	return false;
}
