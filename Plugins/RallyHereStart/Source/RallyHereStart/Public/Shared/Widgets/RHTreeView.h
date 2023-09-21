// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/TreeView.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Slate/SRHTreeView.h"
#include "RHTreeView.generated.h"

/**
 * RHExampleGame extension of UTreeView to fix some interaction discrepancies like double click
 */
UCLASS()
class RALLYHERESTART_API URHTreeView : public UTreeView
{
	GENERATED_BODY()

public:
	URHTreeView(const FObjectInitializer& Initializer)
		: UTreeView(Initializer), MyHud(nullptr)
	{ }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="RH|Tree View")
	void InitializeWidget();
	virtual void InitializeWidget_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="RH|Tree View")
	void UninitializeWidget();
	virtual void UninitializeWidget_Implementation();


	UFUNCTION(BlueprintPure, Category="RH|Tree View")
	inline bool IsItemExpanded(UObject* const Item) const
	{
		if (MyTreeView.IsValid())
		{
			return MyTreeView->IsItemExpanded(Item);
		}
		return false;
	}

	/** Gets the number of items that are affecting the layout of the tree view (i.e. top-level item + sub-items only for expanded sections) */
	UFUNCTION(BlueprintPure, Category="RH|Tree View")
	inline int32 GetNumItemsInLayout() const
	{
		if (auto* View = static_cast<SRHTreeView<UObject*>*>(MyTreeView.Get()))
		{
			return View->GetNumItemsInLayout();
		}
		return 0;
	}

	/** Moves the selection in a way that acts like the user navigated to it (doesn't return to the last user-clicked item when they move) */
	UFUNCTION(BlueprintCallable, Category="RH|Tree View")
	void NavigateSelectItem(UObject* Item)
	{
		if (auto* Tree = MyTreeView.Get())
		{
			Tree->SetSelection(Item, ESelectInfo::OnNavigation);
		}
	}

protected:
	virtual void OnItemDoubleClickedInternal(UObject* ListItem) override;

	virtual TSharedRef<STableViewBase> RebuildListWidget() override;

	UPROPERTY(BlueprintReadOnly, Category = "RH|Tree View")
	TWeakObjectPtr<class ARHHUDCommon> MyHud;

private:
	UFUNCTION(BlueprintCallable, Category="RH|Tree View", meta=(AllowPrivateAccess = true, DisplayName="Get Entry Widget From Item"))
	bool BP_GetEntryWidgetFromItem(const UObject* Item, UUserWidget*& OutWidget) const
	{
		return (OutWidget = GetEntryWidgetFromItem<UUserWidget>(Item)) != nullptr;
	}
};
