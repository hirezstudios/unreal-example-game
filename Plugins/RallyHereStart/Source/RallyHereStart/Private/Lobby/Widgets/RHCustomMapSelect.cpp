#include "Engine/DataTable.h"
#include "Components/UniformGridPanel.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Lobby/Widgets/RHMapButton.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "Lobby/Widgets/RHCustomMapSelect.h"

URHCustomMapSelect::URHCustomMapSelect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	MapButtonClass(URHMapButton::StaticClass()),
	MaxColumn(2)
{
}

void URHCustomMapSelect::OnShown_Implementation()
{
	Super::OnShown_Implementation();

	if (UUniformGridPanel* MapGrid = GetMapGrid())
	{
		MapGrid->ClearChildren();
		if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
		{
			if (UDataTable* MapDetailsDT = QueueDataFactory->GetMapsDetailsDT())
			{
				int32 CurrentRow = 0;
				int32 CurrentCol = 0;

				TArray<FName> MapNames = MapDetailsDT->GetRowNames();
				for (FName MapName : MapNames)
				{
					if (URHMapButton* MapButton = CreateWidget<URHMapButton>(this, MapButtonClass))
					{
						MapButton->InitializeWidget();
						MapButton->SetMap(MapName);
						MapButton->SetSelected(MapName == QueueDataFactory->GetSelectedCustomMap());
						MapButton->OnMapButtonClicked.AddDynamic(this, &URHCustomMapSelect::HandleOnMapButtonSelected);
						MapButton->SetVisibility(ESlateVisibility::Visible);
						MapButton->SetPadding(FMargin(5.0, 5.0, 5.0, 5.0));

						MapGrid->AddChildToUniformGrid(MapButton, CurrentRow, CurrentCol);

						if (++CurrentCol >= MaxColumn)
						{
							CurrentCol = 0;
							++CurrentRow;
						}
					}
				}
			}
		}
	}
}

bool URHCustomMapSelect::NavigateBack_Implementation()
{
	RemoveViewRoute(FName("MapSelect"));
	return Super::NavigateBack_Implementation();
}

void URHCustomMapSelect::HandleOnMapButtonSelected(FName MapName)
{
	if (URHQueueDataFactory* QueueDataFactory = GetQueueDataFactory())
	{
		QueueDataFactory->SetMapForCustomMatch(MapName);
	}
	NavigateBack();
}

URHQueueDataFactory* URHCustomMapSelect::GetQueueDataFactory() const
{
	if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
	{
		return RHLobbyHUD->GetQueueDataFactory();
	}
	return nullptr;
}