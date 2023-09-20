// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "Inventory/IconInfo.h"
#include "RHBanner.generated.h"

/**
 * 
 */
UCLASS(AutoExpandCategories=(SmallBanner,LargeBanner))
class RALLYHERESTART_API URHBanner : public UPlatformInventoryItem
{
	GENERATED_BODY()
	
public:
	URHBanner(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Returns the soft reference to the small banner icon
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UIconInfo* GetSmallBannerInfo() const { return SmallBannerIconInfo; }

	// Returns the soft reference to the large banner icon
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UIconInfo* GetLargeBannerInfo() const { return LargeBannerIconInfo; }

	virtual UIconInfo* GetItemIconInfo() const override { return SmallBannerIconInfo; }

#if WITH_EDITOR
	virtual void GetPreloadDependencies(TArray<UObject*>& OutDeps) override;
#endif

protected:

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Small Banner", meta = (DisplayName = "Small Banner Icon"))
	UIconInfo* SmallBannerIconInfo;

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Large Banner", meta = (DisplayName = "Large Banner Icon"))
	UIconInfo* LargeBannerIconInfo;
};
