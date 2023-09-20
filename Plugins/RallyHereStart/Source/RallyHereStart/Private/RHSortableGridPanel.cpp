// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RHSortableGridPanel.h"

UGridSlot* URHSortableGridPanel::AddChildAutoLayout(UWidget* Content)
{
	int32 PreAddCount = GetChildrenCount();
	UGridSlot* GridSlot = AddChildToGrid(Content);
	if (GridSlot != nullptr)
	{
		if (Orientation == EOrientation::Orient_Horizontal &&
			RowFill.Num() > 0)
		{
			int32 Row = PreAddCount % RowFill.Num();
			int32 Column = PreAddCount / RowFill.Num();
			GridSlot->SetRow(Row);
			GridSlot->SetColumn(Column);
		}
		else if (Orientation == EOrientation::Orient_Vertical &&
				 ColumnFill.Num() > 0)
		{
			int32 Row = PreAddCount / ColumnFill.Num();
			int32 Column = PreAddCount % ColumnFill.Num();
			GridSlot->SetRow(Row);
			GridSlot->SetColumn(Column);
		}
	}
	return GridSlot;
}

void URHSortableGridPanel::SortChildren()
{
	struct FCompareChildren
	{
		const FSortChildrenComparator& ComparatorEvent;
		FCompareChildren(const FSortChildrenComparator& InComparatorEvent) :
			ComparatorEvent(InComparatorEvent)
		{	}

		bool operator()(const UWidget& A, const UWidget& B) const
		{
			return ComparatorEvent.Execute(&A, &B);
		}
	};

	int ChildCount = GetChildrenCount();
	bool Bound = OnSortCompareChildrenEvent.IsBound();
	if (ChildCount > 0 &&
		Bound)
	{
		TArray<UWidget*> Children = GetAllChildren();
		Children.Sort(FCompareChildren(OnSortCompareChildrenEvent));

 		ClearChildren();
 		for (int32 i = 0; i < Children.Num(); i++)
 		{
			AddChildAutoLayout(Children[i]);
 		}
	}
}