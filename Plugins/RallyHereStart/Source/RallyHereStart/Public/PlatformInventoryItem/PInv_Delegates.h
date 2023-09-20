// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

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
