// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "Subsystems/RHStoreSubsystem.h"
#include "Inventory/RHBanner.h"

URHBanner::URHBanner(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	static const FGameplayTag ItemCollectionTag = FGameplayTag::RequestGameplayTag(CollectionNames::BannerCollectionName);
	CollectionContainer.AddTag(ItemCollectionTag);
	
	IsOwnableInventoryItem = true;

	SmallBannerIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("SmallBannerIconInfo"));
	LargeBannerIconInfo = CreateDefaultSubobject<UImageIconInfo>(TEXT("LargeBannerIconInfo"));
}

#if WITH_EDITOR
void URHBanner::GetPreloadDependencies(TArray<UObject*>& OutDeps)
{
	Super::GetPreloadDependencies(OutDeps);

	if (SmallBannerIconInfo != nullptr)
	{
		OutDeps.Add(SmallBannerIconInfo);
	}

	if (LargeBannerIconInfo != nullptr)
	{
		OutDeps.Add(LargeBannerIconInfo);
	}
}
#endif
