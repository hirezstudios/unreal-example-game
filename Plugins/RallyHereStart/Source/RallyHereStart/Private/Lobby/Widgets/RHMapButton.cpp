#include "Lobby/HUD/RHLobbyHUD.h"
#include "DataFactories/RHQueueDataFactory.h"
#include "Lobby/Widgets/RHMapButton.h"

void URHMapButton::SetMap_Implementation(FName InMapName)
{
	MapName = InMapName;
}

bool URHMapButton::GetMapDetails(FRHMapDetails& OutMapDetails) const
{
	if (ARHLobbyHUD* RHLobbyHUD = Cast<ARHLobbyHUD>(MyHud))
	{
		if (URHQueueDataFactory* QueueDataFactory = RHLobbyHUD->GetQueueDataFactory())
		{
			return QueueDataFactory->GetMapDetailsFromRowName(MapName, OutMapDetails);
		}
	}
	return false;
}