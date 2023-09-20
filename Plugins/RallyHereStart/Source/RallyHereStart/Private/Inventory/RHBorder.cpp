// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "Inventory/RHBorder.h"

URHBorder::URHBorder(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	static const FGameplayTag ItemCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::BorderCollectionName);
	CollectionContainer.AddTag(ItemCollectionTag);

	IsOwnableInventoryItem = true;

	SmallBorderIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("SmallBorderIconInfo"));
	LargeBorderIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("LargeBorderIconInfo"));
}

#if WITH_EDITOR
void URHBorder::GetPreloadDependencies(TArray<UObject*>& OutDeps)
{
	Super::GetPreloadDependencies(OutDeps);

	if (SmallBorderIconInfo != nullptr)
	{
		OutDeps.Add(SmallBorderIconInfo);
	}

	if (LargeBorderIconInfo != nullptr)
	{
		OutDeps.Add(LargeBorderIconInfo);
	}
}
#endif
