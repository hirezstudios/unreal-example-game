#include "RallyHereStart.h"
#include "Components/CanvasPanelSlot.h"
#include "Shared/Widgets/RHCanvasPanel.h"

URHCanvasPanel::URHCanvasPanel(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

void URHCanvasPanel::PlaceWidgetUnder(UUserWidget* BottomWidget, UUserWidget* TopWidget)
{
    int32 BottomIndex = Slots.Find(BottomWidget->Slot);
    int32 TopIndex = Slots.Find(TopWidget->Slot);

    if (BottomIndex != INDEX_NONE && TopIndex != INDEX_NONE)
    {
        UCanvasPanelSlot* BottomSlot = Cast<UCanvasPanelSlot>(BottomWidget->Slot);
        UCanvasPanelSlot* TopSlot = Cast<UCanvasPanelSlot>(TopWidget->Slot);

        if (BottomSlot && TopSlot)
        {
            BottomSlot->SetZOrder(TopSlot->GetZOrder() - 1);
        }
    }
}