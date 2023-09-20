// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "Inventory/RHTitle.h"

URHTitle::URHTitle(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectInitializer)
{
    static const FGameplayTag TitleCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::TitleCollectionName);
    CollectionContainer.AddTag(TitleCollectionTag);

    IsOwnableInventoryItem = true;
}