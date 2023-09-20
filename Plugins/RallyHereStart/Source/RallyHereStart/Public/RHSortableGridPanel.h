// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GridPanel.h"
#include "RHSortableGridPanel.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSortableGridPanel : public UGridPanel
{
	GENERATED_BODY()

public:
	URHSortableGridPanel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer),
		Orientation(EOrientation::Orient_Vertical)
	{

	}

	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FSortChildrenComparator, const UWidget*, LHS, const UWidget*, RHS);
	
	//This indicates the direction of layout to automatically apply to the children added to 
	//this grid panel.  The number of rows/columns is pulled from the size of the number of
	//entries in RowFill/ColumnFill
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EOrientation> Orientation;

	/** Called when the widget is needed for the item. */
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSortChildrenComparator OnSortCompareChildrenEvent;
	
	//This will add a child widget to this grid, automatically setting it's row/column based on config
	UFUNCTION(BlueprintCallable, Category = "Widget")
	UGridSlot* AddChildAutoLayout(UWidget* Content);

	UFUNCTION(BlueprintCallable, Category = "Sort")
	void SortChildren();
};
