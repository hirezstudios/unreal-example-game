// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStartItemFactory.h"

#include "Kismet2/SClassPickerDialog.h"

#include "RallyHereStartAssetClassParentFilter.h"
#include "ClassViewerModule.h"

URallyHereStartItemFactory::URallyHereStartItemFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = UPlatformInventoryItem::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool URallyHereStartItemFactory::ConfigureProperties()
{
    const FText TitleText = INVTEXT("Pick Item Class");
    CONFIGURE_PROPERTIES(RHItemClass, UPlatformInventoryItem::StaticClass(), TitleText)
}

UObject* URallyHereStartItemFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UPlatformInventoryItem* NewItem = nullptr;

	if (RHItemClass != nullptr)
	{
		NewItem = NewObject<UPlatformInventoryItem>(InParent, RHItemClass, Name, Flags | RF_Transactional);
	}
	else
	{
		// if we have no data asset class, use the passed-in class instead
		check(Class->IsChildOf(UPlatformInventoryItem::StaticClass()));
		NewItem = NewObject<UPlatformInventoryItem>(InParent, Class, Name, Flags);
	}

	return NewItem;
}