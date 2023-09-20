#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHCustomMapSelect.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHCustomMapSelect : public URHWidget
{
	GENERATED_BODY()

	URHCustomMapSelect(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnShown_Implementation() override;
	virtual bool NavigateBack_Implementation() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Custom Map Select")
    TSubclassOf<class URHMapButton> MapButtonClass;

	/* Max number of columns in the map grid */
	UPROPERTY(EditDefaultsOnly, Category = "Custom Map Select")
	int32 MaxColumn;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Custom Map Select")
	class UUniformGridPanel* GetMapGrid();

private:
	UFUNCTION()
	void HandleOnMapButtonSelected(FName MapName);

	class URHQueueDataFactory* GetQueueDataFactory() const;
};
