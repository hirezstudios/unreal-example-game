// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
*
*/
class RALLYHERESTART_API PInv_Delegates
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnReadyForBundleData);

	DECLARE_MULTICAST_DELEGATE(FOnPostInitialAssetScan);

	// Callback when the asset manager is initialized
	static FOnReadyForBundleData OnReadyForBundleData;

	// Callback when the asset manager finishes its initial asset scan
	static FOnPostInitialAssetScan OnPostInitialAssetScan;

private:
	PInv_Delegates() {}
};
