// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Inventory/IconInfo.h"
#include "RHAsyncImage.h"
#include "PlatformInventoryItem/PInv_AssetManager.h"
#include "PlatformInventoryItem/PInv_Delegates.h"

UIconInfo::UIconInfo(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
}


UImageIconInfo::UImageIconInfo(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
	, IconImage()
{
	IconTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void UImageIconInfo::ApplyToAsyncImage(URHAsyncImage* InImage, bool bMatchSize /*= false*/)
{
	if (InImage != nullptr && !IconImage.IsNull())
	{
		TWeakObjectPtr<URHAsyncImage> WeakImage(InImage); // using weak ptr in case 'InImage' has gone out of scope by the time this lambda is called
		TSoftObjectPtr<UObject> SoftObject(IconImage);
		InImage->SetColorAndOpacity(IconTint);

		InImage->RequestAsyncLoad(SoftObject,
			[WeakImage, SoftObject, bMatchSize]() {
			if (URHAsyncImage* StrongImage = WeakImage.Get())
			{
				UObject* StrongObject = SoftObject.Get();
				ensureMsgf(StrongObject, TEXT("Failed to load %s"), *SoftObject.ToSoftObjectPath().ToString());
				if (UTexture2D* StrongTexture = Cast<UTexture2D>(StrongObject))
				{
					StrongImage->InternalSetBrushFromItemTexture(StrongTexture, bMatchSize);
				}
				else if (UMaterialInterface* StrongMI = Cast<UMaterialInterface>(StrongObject))
				{
					StrongImage->SetBrushFromMaterial(StrongMI);
				}
				else
				{
					ensureMsgf(false, TEXT("Object %s is not a valid object type. Must be Texture2D or MaterialInterface."), *GetNameSafe(StrongObject));
					StrongImage->InternalSetBrushFromItemTexture(nullptr, bMatchSize);
				}
			}
		});
	}
}