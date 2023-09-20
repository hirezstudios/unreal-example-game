// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "Shared/Widgets/RHSocialOverlay.h"
#include "Shared/Social/RHDataSocialCategory.h"
#include "Components/TreeView.h"
#include "RHSocialPanelBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSocialDataReady, const TArray<class URHDataSocialCategory*>&, SortedData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSocialPlayerCardClicked, URHWidget*, PlayerCard);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSocialHeaderClicked, URHWidget*, Header);

UENUM(BlueprintType)
enum class ERHSocialPanelDisplayOption : uint8
{
	HideIfEmpty,
	ShowIfEmpty
};

USTRUCT(BlueprintType)
struct FRHSocialPanelSectionDef
{
	GENERATED_USTRUCT_BODY()

	ERHSocialOverlaySection Section;
	ERHSocialPanelDisplayOption DisplayOption;
	ERHCategoryOpenMode OpenMode;
};

/**
 * 
 */
UCLASS()
class RALLYHERESTART_API URHSocialPanelBase : public URHWidget
{
	GENERATED_BODY()
	
public:
	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

	// Initialize the tree to be able to handle our RHDataSocialCategory as header data
	UFUNCTION(BlueprintCallable, Category = "RH|Social Panel")
	void SetupTreeView(class UTreeView* List);

	UFUNCTION(BlueprintCallable, Category = "RH|Social Panel")
	void SetDataSource(URHSocialOverlay* Source);

	UPROPERTY(BlueprintAssignable, Category = "RH|Social Panel")
	FOnSocialDataReady OnDataReady;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "RH|Social Panel")
	FOnSocialPlayerCardClicked OnPlayerCardClicked;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "RH|Social Panel")
	FOnSocialHeaderClicked OnSocialHeaderClicked;

protected:
	UFUNCTION()
	virtual void OnDataChange(const TArray<FRHSocialOverlaySectionInfo>& Sections = TArray<FRHSocialOverlaySectionInfo>());

	UFUNCTION()
	virtual void UpdateListData();

	UFUNCTION()
	void GetSubListFromData(UObject* Source, TArray<UObject*>& Out_List);

	UFUNCTION(BlueprintPure, Category="RH|Social Panel")
	inline class UTreeView* GetTreeView() const { return TreeView; }

	UFUNCTION(BlueprintPure, Category="RH|Social Panel")
	inline URHSocialOverlay* GetDataSource() const { return DataSource; }

	template<uint32 SIZE>
	inline void SetSections(const FRHSocialPanelSectionDef (&Sections)[SIZE])
	{
		MySections.Append(Sections, SIZE);
	}

private:
	UPROPERTY(Transient)
	class UTreeView* TreeView;
	UPROPERTY(Transient)
	class URHSocialOverlay* DataSource;
	TArray<FRHSocialPanelSectionDef> MySections;
	UPROPERTY(Transient)
	TArray<URHDataSocialCategory*> CategoriesList;
};
