#include "Shared/HUD/RHHUDCommon.h"
#include "Managers/RHViewManager.h"
#include "Game/Widgets/RHGameWidget.h"

URHGameWidget::URHGameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URHGameWidget::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	// Create View Manager
	if (MyHud != nullptr)
	{
		if (URHViewManager* CreatedViewManager = NewObject<URHViewManager>(this))
		{
			CreatedViewManager->SetMembersOnConstruct(MyHud.Get(), GetPanelsForViewManager(), GetStickyWidgetDataForViewManager(), GameViewTable, true);
			MyHud->SetViewManager(CreatedViewManager);
			CreatedViewManager->Initialize();
		}
	}
}

void URHGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Action bindings
	ScoreboardPressed.BindDynamic(this, &URHGameWidget::HandleScoreboardPressed);
	ListenForInputAction(FName("Scoreboard"), EInputEvent::IE_Pressed, true, ScoreboardPressed);
}

void URHGameWidget::HandleScoreboardPressed()
{
	AddViewRoute(FName("Scoreboard"));
}