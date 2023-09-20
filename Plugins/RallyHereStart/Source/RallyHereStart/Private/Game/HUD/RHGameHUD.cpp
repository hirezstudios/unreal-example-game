#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Game/HUD/RHGameHUD.h"

ARHGameHUD::ARHGameHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	GameHUDWidgetClass(URHWidget::StaticClass())
{
}

void ARHGameHUD::BeginPlay()
{
	Super::BeginPlay();

	if (URHWidget* CreatedHUDWidget = CreateWidget<URHWidget>(GetOwningPlayerController(), GameHUDWidgetClass))
	{
		CreatedHUDWidget->AddToPlayerScreen();
		CreatedHUDWidget->InitializeWidget();

		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(GetOwningPlayerController(), CreatedHUDWidget);

		GameHUDWidget = CreatedHUDWidget;
		OnGameHUDWidgetCreated();
	}
}

void ARHGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GameHUDWidget)
	{
		GameHUDWidget->RemoveFromParent();
	}

	Super::EndPlay(EndPlayReason);
}