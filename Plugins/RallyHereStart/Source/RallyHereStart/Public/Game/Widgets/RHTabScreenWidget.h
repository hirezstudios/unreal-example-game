#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHTabScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHTabScreenWidget : public URHWidget
{
	GENERATED_BODY()

public:
	URHTabScreenWidget(const FObjectInitializer& ObjectInitializer);
	
	virtual void InitializeWidget_Implementation();
	virtual void OnShown_Implementation() override;
	virtual void OnHide_Implementation() override;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Screen Widget")
	void ClearScoreboard();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Screen Widget")
	void AddPlayerToScoreboard(int32 TeamNum, const FString& PlayerName);

#pragma region INTERACTIONS
protected:
	FOnInputAction ScoreboardReleased;

	UFUNCTION()
	void HandleScoreboardReleased();
#pragma endregion
};
