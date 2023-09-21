// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RHSortableVerticalBox.h"

void URHSortableVerticalBox::SortChildren()
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
 			AddChildToVerticalBox(Children[i]);
 		}
	}
}