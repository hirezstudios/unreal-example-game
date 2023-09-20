// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Social/RHSocialPanelBase.h"
#include "Shared/Widgets/RHSocialOverlay.h"
#include "Shared/Social/RHDataSocialCategory.h"
#include "Components/TreeView.h"
#include "Templates/RemoveReference.h"
#include "Algo/Partition.h"
#include "Algo/Find.h"
#include "Algo/RemoveIf.h"

void URHSocialPanelBase::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();
}

void URHSocialPanelBase::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();

	if (DataSource != nullptr)
	{
		DataSource->OnDataChanged.RemoveDynamic(this, &URHSocialPanelBase::OnDataChange);
	}
}
void URHSocialPanelBase::SetupTreeView(UTreeView* List)
{
	TreeView = List;
	TreeView->SetOnGetItemChildren(this, &URHSocialPanelBase::GetSubListFromData);
}

void URHSocialPanelBase::SetDataSource(URHSocialOverlay* Source)
{
	if (DataSource != nullptr)
	{
		DataSource->OnDataChanged.RemoveDynamic(this, &URHSocialPanelBase::OnDataChange);
	}

	DataSource = Source;

	if (DataSource != nullptr)
	{
		DataSource->OnDataChanged.AddDynamic(this, &URHSocialPanelBase::OnDataChange);
	}

	OnDataChange();
}

void URHSocialPanelBase::GetSubListFromData(UObject* Source, TArray<UObject*>& Out_List)
{
	if (auto* Category = Cast<URHDataSocialCategory>(Source))
	{
		// Easiest way to copy arrays of polymorphically compatible types - create a new array using templated constructor + move
		using TargetArrayType = TRemoveReference<decltype(Out_List)>::Type;
		Out_List = TargetArrayType(Category->GetPlayerList());
	}
	else
	{
		Out_List.Empty();
	}
}

void URHSocialPanelBase::OnDataChange(const TArray<FRHSocialOverlaySectionInfo>& Sections)
{
	if (DataSource == nullptr)
	{
		UE_LOG(RallyHereStart, Warning, TEXT("RHSocialPanelBase::OnDataChange Ignoring due to missing data source"));
		return;
	}

	auto& SectionDefs = MySections;

	auto ShouldShowCategory = [&SectionDefs](URHDataSocialCategory* Category)
	{
		auto* SectionDef = Algo::FindByPredicate(SectionDefs, [Category](const FRHSocialPanelSectionDef& Def)
		{
			return Def.Section == Category->GetSectionValue<ERHSocialOverlaySection>();
		});

		if (SectionDef != nullptr)
		{
			return SectionDef->DisplayOption == ERHSocialPanelDisplayOption::ShowIfEmpty
				|| Category->GetPlayerList().Num() > 0
				|| !Category->GetMessageText().IsEmpty()
				|| Category->IsProcessing();
		}
		return false;
	};

	auto& Categories = DataSource->GetData();
	if (Sections.Num() == 0) // update all
	{
		CategoriesList.Empty(MySections.Num());

		for (auto* Category : Categories)
		{
			if (ShouldShowCategory(Category))
			{
				CategoriesList.Emplace(Category);
			}
		}
	}
	else
	{
		for (auto Section : Sections)
		{
			auto* Category = DataSource->GetCategory(Section);
			if (Category != nullptr)
			{
				if (!ShouldShowCategory(Category))
				{
					for (int32 i = 0; i < CategoriesList.Num(); ++i)
					{
						if (CategoriesList[i]->GetSectionValue<ERHSocialOverlaySection>() == Section.Section)
						{
							CategoriesList.RemoveAt(i);
							break;
						}
					}
				}
				else
				{
					CategoriesList.AddUnique(Category);
				}
			}
		}
	}

	Algo::Sort(CategoriesList, &RHSocial::CategorySort);
	UpdateListData();
}


void URHSocialPanelBase::UpdateListData()
{
	auto* List = GetTreeView();
	if (List == nullptr)
	{
		return;
	}
	List->SetListItems(CategoriesList);

	for (auto& Item : CategoriesList)
	{
		if (Item->TryConsumeOpenOnUpdate())
		{
			List->SetItemExpansion(Item, true);
		}
	}
}