// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/PrimaryAssetId.h"
#include "PInv_AssetManagerSettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, defaultconfig, notplaceable, meta=(DisplayName="Platform Asset Manager"))
class RALLYHERESTART_API UPInv_AssetManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPInv_AssetManagerSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** cook rules Unknown unless otherwise specified. */
	UPROPERTY(config, EditAnywhere, Category="Quick Cook")
	bool bQuickCook;

	/** Primary Asset Types that keep their cook rules during a quick cook */
	UPROPERTY(config, EditAnywhere, Category = "Quick Cook")
	TArray<FName> PrimaryTypesToIgnoreQuickCook;

	UPROPERTY(Transient)
	TSet<FName> QuickCookTypeIgnoreSet;

	/** Specific primary assets that keep their custom cook rules during a quick cook */
	UPROPERTY(config, EditAnywhere, Category = "Quick Cook")
	TArray<FString> PrimaryAssetsToIgnoreQuickCook;

	UPROPERTY(Transient)
	TSet<FPrimaryAssetId> QuickCookAssetIgnoreSet;
	UPROPERTY(Transient)
	TSet<FPrimaryAssetType> QuickCookAssetIngoreSetTypes;

	/** Specific primary assets to include in a quick cook */
	UPROPERTY(config, EditAnywhere, Category = "Quick Cook")
	TArray<FString> PrimaryAssetsToIncludeQuickCook;

#if WITH_EDITOR
	bool IgnoreQuickCookRulesByType(const FPrimaryAssetType& InType) const;
	bool IgnoreQuickCookRulesByAsset(const FPrimaryAssetId& InAsset) const;

	void RebuildQuickCookSets();

	virtual void PostReloadConfig(class FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


    UPROPERTY(config, EditAnywhere, Category="Cook Profile")
    FName CookProfile;
};
