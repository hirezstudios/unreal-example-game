#pragma once
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerController.h"
#include "RHGameState.generated.h"


UCLASS(config=Game)
class RALLYHERESTART_API ARHGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

public:
    virtual void AddPlayerState(APlayerState* PlayerState) override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;
};