#pragma once

#include "Widgets/Views/STreeView.h"
#include "RHUIBlueprintFunctionLibrary.h"

/**
 * RHExampleGame extension of STreeView to add some more accessors
 */
template<typename ItemType>
class SRHTreeView : public STreeView<ItemType>
{
public:
	using NullableItemType = typename STreeView<ItemType>::NullableItemType;

	/** Returns the number of items that currently affect the view space. i.e. Top Level Items + sub-items in expanded sections */
	inline int32 GetNumItemsInLayout() const
	{
		return this->LinearizedItems.Num();
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		FKey FakeKey = EKeys::Invalid;
		NullableItemType ItemToSelect = nullptr;

		if (InKeyEvent.GetKey() == EKeys::Gamepad_DPad_Left)
		{
			FakeKey = EKeys::Left;
		}
		else if (InKeyEvent.GetKey() == EKeys::Gamepad_DPad_Right)
		{
			FakeKey = EKeys::Right;
		}
		else if (InKeyEvent.GetKey() == EKeys::Gamepad_DPad_Up || InKeyEvent.GetKey() == EKeys::Gamepad_LeftStick_Up)
		{
			// Move up from the selection range start
			if (this->RangeSelectionStart == nullptr)
			{
				if (this->LinearizedItems.Num() > 0)
				{
					ItemToSelect = this->LinearizedItems[0];
				}
			}
			else
			{
				int32 Index = this->LinearizedItems.Find(this->RangeSelectionStart);
				if (Index > 0)
				{
					ItemToSelect = this->LinearizedItems[Index - 1];
				}
			}
		}
		else if (InKeyEvent.GetKey() == EKeys::Gamepad_DPad_Down || InKeyEvent.GetKey() == EKeys::Gamepad_LeftStick_Down)
		{
			// Move down from the selection range end
			if (this->SelectorItem == nullptr)
			{
				if (this->LinearizedItems.Num() > 0)
				{
					ItemToSelect = this->LinearizedItems[0];
				}
			}
			else
			{
				int32 Index = this->LinearizedItems.Find(this->SelectorItem);
				ItemToSelect = this->LinearizedItems[FMath::Min(Index + 1, this->LinearizedItems.Num() - 1)];
			}
		}

		if (FakeKey != EKeys::Invalid)
		{
			FKeyEvent FakeEvent = FKeyEvent(
				FakeKey,
				InKeyEvent.GetModifierKeys(),
				InKeyEvent.GetUserIndex(),
				InKeyEvent.IsRepeat(),
				InKeyEvent.GetCharacter(),
				InKeyEvent.GetKeyCode()
			);

			return STreeView<ItemType>::OnKeyDown(MyGeometry, FakeEvent);
		}
		else if (ItemToSelect != nullptr)
		{
			this->NavigationSelect(ItemToSelect, InKeyEvent);
			return FReply::Handled();
		}
		else
		{
			return STreeView<ItemType>::OnKeyDown(MyGeometry, InKeyEvent);
		}
	}

	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (TListTypeTraits<ItemType>::IsPtrValid(this->SelectorItem) && InKeyEvent.GetKey() == URHUIBlueprintFunctionLibrary::GetGamepadConfirmButton())
		{
			this->Private_OnItemClicked(this->SelectorItem);
		}
		return STreeView<ItemType>::OnKeyUp(MyGeometry, InKeyEvent);
	}
};