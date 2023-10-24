#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHHomeScreenWidget.generated.h"

UCLASS()
class RALLYHERESTART_API URHHomeScreenWidget : public URHWidget
{
    GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void CheckForOnShownEvents();

	bool CheckForVoucherRedemption();

	bool CheckForWhatsNewModal();

	//$$ KAB - Route names changed to Gameplay Tags, New Var to set ViewTag
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Views", meta = (Categories = "View"))
	FGameplayTag NewsViewTag;
};