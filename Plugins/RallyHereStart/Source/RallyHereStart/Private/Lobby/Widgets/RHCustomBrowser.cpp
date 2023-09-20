// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "Lobby/Widgets/RHCustomBrowserEntry.h"
#include "Lobby/Widgets/RHCustomBrowser.h"

URHCustomBrowser::URHCustomBrowser(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	BrowserEntryWidgetClass(URHCustomBrowserEntry::StaticClass())
{
}

void URHCustomBrowser::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();
	if (auto QueueDataFactory = GetQueueDataFactory())
	{
		QueueDataFactory->OnCustomSearchResultReceived.AddDynamic(this, &URHCustomBrowser::HandleSearchResultReceived);
	}
}

void URHCustomBrowser::UninitializeWidget_Implementation()
{
	if (auto QueueDataFactory = GetQueueDataFactory())
	{
		QueueDataFactory->OnCustomSearchResultReceived.RemoveDynamic(this, &URHCustomBrowser::HandleSearchResultReceived);
	}
	Super::UninitializeWidget_Implementation();
}

void URHCustomBrowser::OnShown_Implementation()
{
	Super::OnShown_Implementation();
	if (auto QueueDataFactory = GetQueueDataFactory())
	{
		QueueDataFactory->DoSearchForCustomGames();
	}
}

void URHCustomBrowser::HandleSearchResultReceived(TArray<URH_SessionView*> CustomSessions)
{
	DisplayResult(CustomSessions);
}

void URHCustomBrowser::DisplayResult_Implementation(const TArray<URH_SessionView*>& CustomSessions)
{
	if (UPanelWidget* Container = GetBrowserEntriesContainer())
	{
		Container->ClearChildren();
		for (URH_SessionView* Session : CustomSessions)
		{
			if (Session != nullptr)
			{
				if (URHCustomBrowserEntry* EntryWidget = CreateWidget<URHCustomBrowserEntry>(this, BrowserEntryWidgetClass))
				{
					EntryWidget->InitializeWidget();
					EntryWidget->SetCustomGameInfo(Session);
					Container->AddChild(EntryWidget);
				}
			}
		}
	}
}

URHQueueDataFactory* URHCustomBrowser::GetQueueDataFactory() const
{
	if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
	{
		return RHLobbyHUD->GetQueueDataFactory();
	}
	return nullptr;
}