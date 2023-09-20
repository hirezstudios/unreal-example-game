// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/IconInfo.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RHAvatar.generated.h"

/**
 * 
 */
UCLASS(AutoExpandCategories=(LargeAvatar))
class RALLYHERESTART_API URHAvatar : public UPlatformInventoryItem
{
	GENERATED_BODY()
	
public:
	URHAvatar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE UIconInfo* GetLargeAvatarInfo() const { return LargeAvatarIconInfo; }

	virtual UIconInfo* GetItemIconInfo() const override { return LargeAvatarIconInfo; }

#if WITH_EDITOR
	virtual void GetPreloadDependencies(TArray<UObject*>& OutDeps) override;
#endif

protected:

	UPROPERTY(EditAnywhere, NoClear, Instanced, Category="Large Avatar", meta=(DisplayName="Large Avatar Icon"))
	UIconInfo* LargeAvatarIconInfo;
};
