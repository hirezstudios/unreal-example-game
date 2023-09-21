// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "Inventory/RHTitle.h"

URHTitle::URHTitle(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
    : Super(ObjectInitializer)
{
    static const FGameplayTag TitleCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::TitleCollectionName);
    CollectionContainer.AddTag(TitleCollectionTag);

    IsOwnableInventoryItem = true;
}