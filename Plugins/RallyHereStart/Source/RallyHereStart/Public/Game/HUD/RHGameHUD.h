#pragma once

#include "CoreMinimal.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RHGameHUD.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API ARHGameHUD : public ARHHUDCommon
{
	GENERATED_BODY()

public:
	ARHGameHUD(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game HUD")
    TSubclassOf<class URHWidget> GameHUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
	URHWidget* GameHUDWidget;

	UFUNCTION(BlueprintImplementableEvent, Category = "Game HUD")
	void OnGameHUDWidgetCreated();
};
