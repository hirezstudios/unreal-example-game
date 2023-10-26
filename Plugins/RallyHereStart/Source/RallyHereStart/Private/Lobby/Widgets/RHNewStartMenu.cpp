// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Subsystems/RHNewsSubsystem.h"
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
	if (URHNewsSubsystem* pNewsSubsystem = GetNewsSubsystem())
	{
		TArray<URHJsonData*> DataToCheck;

		TSharedPtr<FJsonObject> LandingPanelJson = pNewsSubsystem->GetJsonPanelByName(TEXT("landingpanel"));

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

									pNewsSubsystem->LoadData(Panel, WhatsNewPanelObj);
									if ((*WhatsNewPanelObj)->TryGetObjectField(TEXT("imageUrl"), ObjectField))
									{
										Panel->Image = pNewsSubsystem->GetTextureByRemoteURL(ObjectField);

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

		pNewsSubsystem->CheckShouldShowForPlayer(DataToCheck, MyHud->GetLocalPlayerInfo(), FOnGetShouldShowPanels::CreateLambda([this, Delegate](FRHShouldShowPanelsWrapper Wrapper)
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

URHNewsSubsystem* URHNewStartMenuWidget::GetNewsSubsystem()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<URHNewsSubsystem>();
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("URHNewStartMenuWidget::GetNewsSubsystem Warning: GameInstance  is not currently valid."));
	}
	return nullptr;
}
