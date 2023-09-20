// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/VerticalBox.h"
#include "RHSortableVerticalBox.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSortableVerticalBox : public UVerticalBox
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FSortChildrenComparator, const UWidget*, LHS, const UWidget*, RHS);
	
	/** Called when the widget is needed for the item. */
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSortChildrenComparator OnSortCompareChildrenEvent;

	UFUNCTION(BlueprintCallable, Category = "Sort")
	void SortChildren();
};
