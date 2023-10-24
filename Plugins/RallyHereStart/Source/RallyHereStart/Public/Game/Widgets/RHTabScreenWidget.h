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

	//$$ KAB - Route names changed to Gameplay Tags, New Var to set ViewTag
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Views", meta = (Categories = "View"))
	FGameplayTag ScoreboardViewTag;

	UFUNCTION()
	void HandleScoreboardReleased();
#pragma endregion
};
