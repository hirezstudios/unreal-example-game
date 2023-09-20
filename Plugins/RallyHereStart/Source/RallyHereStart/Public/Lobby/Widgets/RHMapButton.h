#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHMapButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapButtonClicked, FName, MapName);

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHMapButton : public URHWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Map Button")
	void SetMap(FName InMapName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Map Button")
	void SetSelected(bool bSelected);

	UPROPERTY(BlueprintAssignable, Category = "Map Button")
	FOnMapButtonClicked OnMapButtonClicked;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Map Button")
	FName MapName;

	UFUNCTION(BlueprintCallable, Category = "Map Button")
	void OnButtonClicked()
	{
		OnMapButtonClicked.Broadcast(MapName);
	};

	UFUNCTION(BlueprintPure, Category = "Map Button")
	bool GetMapDetails(FRHMapDetails& OutMapDetails) const;
};
