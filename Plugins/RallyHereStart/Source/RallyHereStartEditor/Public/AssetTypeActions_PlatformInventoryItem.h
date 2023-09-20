// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "AssetTypeActions_Base.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"

class FAssetTypeActions_PlatformInventoryItem : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_PlatformInventoryItem(EAssetTypeCategories::Type InAssetCategory) { MyAssetCategory = InAssetCategory; }

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return INVTEXT("Item"); }
	virtual FColor GetTypeColor() const override { return FColor(201, 29, 85); }
	virtual UClass* GetSupportedClass() const override { return UPlatformInventoryItem::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }
	
private:
	EAssetTypeCategories::Type MyAssetCategory;
};