// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "GameFramework/PlayerController.h"
#include "Shared/Widgets/RHTreeView.h"
#include "Components/InputComponent.h"

void URHTreeView::InitializeWidget_Implementation()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		MyHud = PC->GetHUD<ARHHUDCommon>();
	}
}

void URHTreeView::UninitializeWidget_Implementation()
{

}

void URHTreeView::OnItemDoubleClickedInternal(UObject* ListItem)
{
	if (ensure(MyTreeView.IsValid()))
	{
		// The item was clicked, implying that there should certainly be a widget representing this item right now
		TSharedPtr<ITableRow> RowWidget = MyTreeView->WidgetFromItem(ListItem);
		if (ensure(RowWidget.IsValid()) && RowWidget->DoesItemHaveChildren() > 0)
		{
			const bool bNewExpansionState = !MyTreeView->IsItemExpanded(ListItem);
			MyTreeView->SetItemExpansion(ListItem, bNewExpansionState);
		}
	}
	Super::OnItemDoubleClickedInternal(ListItem);
}


TSharedRef<STableViewBase> URHTreeView::RebuildListWidget()
{
	return ConstructTreeView<SRHTreeView>();
}