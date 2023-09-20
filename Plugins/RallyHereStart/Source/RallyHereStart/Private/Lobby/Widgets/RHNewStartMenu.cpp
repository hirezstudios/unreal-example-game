// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Managers/RHJsonDataFactory.h"
#include "Lobby/Widgets/RHNewStartMenu.h"

void URHNewStartMenuWidget::InitializeWidget_Implementation()
{
    Super::InitializeWidget_Implementation();
}

void URHNewStartMenuWidget::UninitializeWidget_Implementation()
{
    Super::UninitializeWidget_Implementation();
}

void URHNewStartMenuWidget::CheckIsNewsAvailable(FOnGetIsNewsAvailableBlock Delegate /*= FOnGetIsNewsAvailableBlock()*/)
{
	if (URHJsonDataFactory* pJsonDataFactory = GetJsonDataFactory())
	{
		TArray<URHJsonData*> DataToCheck;

		TSharedPtr<FJsonObject> LandingPanelJson = pJsonDataFactory->GetJsonPanelByName(TEXT("landingpanel"));

		if (LandingPanelJson.IsValid())
		{
			const TSharedPtr<FJsonObject>* WhatsNewObj;

			if (LandingPanelJson.Get()->TryGetObjectField(TEXT("whatsNewPanel"), WhatsNewObj))
			{
				const TArray<TSharedPtr<FJsonValue>>* WhatsNewPanelContents;

				if ((*WhatsNewObj)->TryGetArrayField(TEXT("content"), WhatsNewPanelContents))
				{
					TArray<TSharedPtr<FJsonValue>> PanelContents = *WhatsNewPanelContents;

					if ((*WhatsNewObj)->TryGetArrayField(TEXT("content"), WhatsNewPanelContents))
					{
						for (TSharedPtr<FJsonValue> WhatsNewPanels : PanelContents)
						{
							const TSharedPtr<FJsonObject>* WhatsNewPanelObj;

							if (WhatsNewPanels->TryGetObject(WhatsNewPanelObj))
							{
								if (URHNewStartMenuData* Panel = NewObject<URHNewStartMenuData>())
								{
									const TSharedPtr<FJsonObject>* ObjectField;

									pJsonDataFactory->LoadData(Panel, WhatsNewPanelObj);
									if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("imageUrl"), ObjectField))
									{
										Panel->Image = pJsonDataFactory->GetTextureByRemoteURL(ObjectField);

										if (Panel->Image)
										{
											DataToCheck.Add(Panel);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		pJsonDataFactory->CheckShouldShowForPlayer(DataToCheck, MyHud->GetLocalPlayerInfo(), FOnGetShouldShowPanels::CreateLambda([this, Delegate](FRHShouldShowPanelsWrapper Wrapper)
			{
				for (const auto& pair : Wrapper.ShouldShowByPanel)
				{
					if (pair.Value) // if any one panel should show, then there is news available
					{
						Delegate.ExecuteIfBound(true);
						return;
					}
				}

				Delegate.ExecuteIfBound(false);
			}));
	}
}

URHJsonDataFactory* URHNewStartMenuWidget::GetJsonDataFactory()
{
	if (MyHud.IsValid())
	{
		if (ARHLobbyHUD* LobbyHud = Cast<ARHLobbyHUD>(MyHud))
		{
			return Cast<URHJsonDataFactory>(LobbyHud->GetJsonDataFactory());
		}
		else
		{
			UE_LOG(RallyHereStart, Warning, TEXT("URHNewStartMainWidget::GetJsonDataFactory Warning: MyHud failed to cast to ARHLobbyHUD."));
		}
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHNewStartMainWidget::GetJsonDataFactory Warning: MyHud is not currently valid."));
	}

	return nullptr;
}
