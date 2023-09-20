#include "RallyHereStart.h"
#include "GameFramework/RHGameState.h"
#include "Player/Controllers/RHPlayerController.h"
#include "GameFramework/PlayerState.h"

ARHGameState::ARHGameState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void ARHGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);

    if (UWorld* CurWorld = GetWorld())
    {
        for (FConstPlayerControllerIterator Iterator = CurWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            ARHPlayerController* RHPlayerController = Cast<ARHPlayerController>(Iterator->Get());
            if (RHPlayerController)
            {
                RHPlayerController->RequestUpdateSonyMatchState(ESonyMatchState::Playing);
            }
        }
    }
}

void ARHGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    if (UWorld* CurWorld = GetWorld())
    {
        for (FConstPlayerControllerIterator Iterator = CurWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            ARHPlayerController* RHPlayerController = Cast<ARHPlayerController>(Iterator->Get());
            if (RHPlayerController)
            {
                RHPlayerController->RequestUpdateSonyMatchState(ESonyMatchState::Playing);
            }
        }
    }
}