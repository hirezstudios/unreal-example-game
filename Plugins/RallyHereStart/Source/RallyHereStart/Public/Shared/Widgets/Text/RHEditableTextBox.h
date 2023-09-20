// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/EditableTextBox.h"
#include "RHEditableTextBox.generated.h"

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHEditableTextBox : public UEditableTextBox
{
	GENERATED_BODY()

public:

    DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FEventReply, FOnEditableTextBoxKeyDownEvent, const FGeometry&, MyGeometry, const FKeyEvent&, InKeyEvent);

public:

    /** Called whenever a key down event happens while the text box is being edited. */
    UPROPERTY(EditAnywhere, Category = "TextBox|Event", meta = (IsBindableEvent = "True"))
    FOnEditableTextBoxKeyDownEvent OnKeyDown;
    
protected:
    //~ Begin UWidget Interface
    virtual TSharedRef<SWidget> RebuildWidget() override;
    // End of UWidget

    virtual FReply HandleOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
};
