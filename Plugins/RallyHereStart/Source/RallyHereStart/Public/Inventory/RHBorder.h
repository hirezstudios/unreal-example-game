// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/IconInfo.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHBorder.generated.h"

/**
 * 
 */
UCLASS(AutoExpandCategories=(SmallBorder,LargeBorder))
class RALLYHERESTART_API URHBorder : public UPlatformInventoryItem
{
	GENERATED_BODY()

public:
	URHBorder(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Returns the soft reference to the small Border icon
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UIconInfo* GetSmallBorderInfo() const { return SmallBorderIconInfo; }

	// Returns the soft reference to the large Border icon
	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UIconInfo* GetLargeBorderInfo() const { return LargeBorderIconInfo; }

	virtual UIconInfo* GetItemIconInfo() const override { return SmallBorderIconInfo; }

#if WITH_EDITOR
	virtual void GetPreloadDependencies(TArray<UObject*>& OutDeps) override;
#endif

protected:

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Small Border", meta = (DisplayName = "Small Border Icon"))
	UIconInfo* SmallBorderIconInfo;

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category = "Large Border", meta = (DisplayName = "Large Border Icon"))
	UIconInfo* LargeBorderIconInfo;
};
