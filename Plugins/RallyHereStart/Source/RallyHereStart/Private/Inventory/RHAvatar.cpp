// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Managers/RHStoreItemHelper.h"
#include "Inventory/RHAvatar.h"

URHAvatar::URHAvatar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	static const FGameplayTag ItemCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::AvatarCollectionName);
	CollectionContainer.AddTag(ItemCollectionTag);

	IsOwnableInventoryItem = true;

	LargeAvatarIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("LargeAvatarIconInfo"));
}

#if WITH_EDITOR
void URHAvatar::GetPreloadDependencies(TArray<UObject*>& OutDeps)
{
	Super::GetPreloadDependencies(OutDeps);

	if (LargeAvatarIconInfo != nullptr)
	{
		OutDeps.Add(LargeAvatarIconInfo);
	}
}
#endif

