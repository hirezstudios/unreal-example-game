#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHGameWidget : public URHWidget
{
	GENERATED_BODY()
	
public:
	URHGameWidget(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWidget_Implementation() override;

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Widget")
	class UDataTable* GameViewTable;

	/* Override in BP */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Widget")
	TArray<UCanvasPanel*> GetPanelsForViewManager() const;

	/* Override in BP */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Widget")
	TArray<FStickyWidgetData> GetStickyWidgetDataForViewManager() const;

#pragma region INTERACTIONS
	FOnInputAction ScoreboardPressed;

	//$$ KAB - Route names changed to Gameplay Tags, New Var to set ViewTag
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Views", meta = (Categories = "View"))
	FGameplayTag ScorboardViewTag;

	UFUNCTION()
	void HandleScoreboardPressed();
#pragma endregion
};
