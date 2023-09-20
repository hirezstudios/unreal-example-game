#pragma once
#include "Components/CanvasPanel.h"
#include "RHCanvasPanel.generated.h"

UCLASS()
class RALLYHERESTART_API URHCanvasPanel : public UCanvasPanel
{
    GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Platform UMG | Canvas Panel")
    void PlaceWidgetUnder(UUserWidget* BottomWidget, UUserWidget* TopWidget);
};