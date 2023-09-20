// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RHAsyncImage.h"
#include "Widgets/Images/SImage.h"
#include "Inventory/IconInfo.h"

const FName URHAsyncImage::DefaultTextureParameterName("Texture");

URHAsyncImage::URHAsyncImage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, WaitingWidget()
{
	MaterialParameter = DefaultTextureParameterName;
}

void URHAsyncImage::ShowWaitingWidget_Implementation()
{
	if (WaitingWidget != nullptr)
	{
		WaitingWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void URHAsyncImage::HideWaitingWidget_Implementation()
{
	if (WaitingWidget != nullptr)
	{
		WaitingWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void URHAsyncImage::SetWaitingWidget(UWidget* InWaitingWidget)
{
	// Confirm WaitingWidget is changing
	if (InWaitingWidget != WaitingWidget)
	{
		// Finish using previous WaitingWidget
		HideWaitingWidget();

		// Set
		WaitingWidget = InWaitingWidget;

		// Jump into the correct visibility state
		if (StreamingHandle.IsValid())
		{
			ShowWaitingWidget();
		}
		else
		{
			HideWaitingWidget();
		}
	}
}


void URHAsyncImage::SetBrushFromSoftPath(const FSoftObjectPath& SoftPath, bool bMatchSize)
{
	TWeakObjectPtr<UImage> WeakThis(this); // using weak ptr in case 'this' has gone out of scope by the time this lambda is called
	TSoftObjectPtr<UObject> SoftObject(SoftPath);

	RequestAsyncLoad(SoftObject,
		[WeakThis, SoftObject, bMatchSize]() {
			if (UImage* StrongThis = WeakThis.Get())
			{
				ensureMsgf(SoftObject.Get(), TEXT("Failed to load %s"), *SoftObject.ToSoftObjectPath().ToString());
				if (UTexture2D* AsTexture = Cast<UTexture2D>(SoftObject.Get()))
				{
					StrongThis->SetBrushFromTexture(AsTexture, bMatchSize);
				}
				else if (UMaterialInterface* AsMaterialInterface = Cast<UMaterialInterface>(SoftObject.Get()))
				{
					StrongThis->SetBrushFromMaterial(AsMaterialInterface);
				}
				else
				{
					ensureMsgf(false, TEXT("Object %s is not a valid object type. Must be Texture2D or MaterialInterface."), *GetNameSafe(SoftObject.Get()));
					StrongThis->SetBrushFromTexture(nullptr, bMatchSize);
				}

			}
		}
	);
}

void URHAsyncImage::SetBrushFromIconInfo(UIconInfo* IconInfo, bool bMatchSize /*= false*/)
{
	if (IconInfo != nullptr)
	{
		IconInfo->ApplyToAsyncImage(this, bMatchSize);
	}
}

void URHAsyncImage::InternalSetBrushFromItemTexture(UTexture2D* StrongTexture, bool bMatchSize)
{
	if (MaterialToUse != nullptr)
	{
		MaterialToUse->SetTextureParameterValue(MaterialParameter, StrongTexture);

		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// This section is copied from "void UImage::SetBrushFromTexture(UTexture2D* Texture, bool bMatchSize)"
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		if (StrongTexture) // Since this texture is used as UI, don't allow it affected by budget.
		{
			StrongTexture->bForceMiplevelsToBeResident = true;
			StrongTexture->bIgnoreStreamingMipBias = true;
		}

		if (bMatchSize)
		{
			FSlateBrush CurrentBrush = GetBrush();
			if (StrongTexture)
			{
				CurrentBrush.ImageSize.X = StrongTexture->GetSizeX();
				CurrentBrush.ImageSize.Y = StrongTexture->GetSizeY();
			}
			else
			{
				CurrentBrush.ImageSize = FVector2D(0, 0);
			}
			SetBrush(CurrentBrush);
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////

		if (GetBrush().GetResourceObject() != MaterialToUse)
		{
			SetBrushFromMaterial(MaterialToUse);
		}
		else
		{
			CancelImageStreaming();

			if (MyImage.IsValid())
			{
#if RH_FROM_ENGINE_VERSION(5,2)
				MyImage->InvalidateImage();
#else
				MyImage->SetImage(&Brush);
#endif
			}

			HideWaitingWidget();
			OnAsyncImageBrushChanged.Broadcast(this);
		}
	}
	else
	{
		SetBrushFromTexture(StrongTexture, bMatchSize);
	}
}

void URHAsyncImage::SetMaterialToUse(UMaterialInstanceDynamic* InMID)
{
	if (InMID == MaterialToUse)
	{
		return;
	}

	UMaterialInstanceDynamic* OldMID = MaterialToUse;
	MaterialToUse = InMID;

	if (OldMID != nullptr)
	{
		if (MaterialToUse == nullptr)
		{
			if (!StreamingHandle.IsValid() || !StreamingHandle->IsActive())
			{
				// Attempt to transfer the existing texture to the Brush (if no update is in progress).
				UTexture* pTextureFromOld = nullptr;
				if (OldMID->GetTextureParameterValue(MaterialParameter, pTextureFromOld, true) && pTextureFromOld != nullptr && pTextureFromOld->IsA<UTexture2D>())
				{
					SetBrushFromTexture(Cast<UTexture2D>(pTextureFromOld), true);
				}
				else if (GetBrush().GetResourceObject() == OldMID)
				{
					SetBrushFromTexture(nullptr);
				}
			}
		}
		else if (!StreamingHandle.IsValid() || !StreamingHandle->IsActive())
		{
			// Attempt to transfer the existing texture to the new MID (if no update is in progress).
			UTexture* pTextureFromOld = nullptr;
			if (OldMID->GetTextureParameterValue(MaterialParameter, pTextureFromOld, true) && pTextureFromOld != nullptr && pTextureFromOld->IsA<UTexture2D>())
			{
				MaterialToUse->SetTextureParameterValue(MaterialParameter, pTextureFromOld);
				SetBrushFromMaterial(MaterialToUse);
			}
		}
	}
	else if (MaterialToUse != nullptr && (!StreamingHandle.IsValid() || !StreamingHandle->IsActive()))
	{
		//Move the existing Texture on the Brush before applying the material to use.
		UTexture2D* pOldBrushTexture = Cast<UTexture2D>(GetBrush().GetResourceObject());
		if (pOldBrushTexture != nullptr)
		{
			MaterialToUse->SetTextureParameterValue(MaterialParameter, pOldBrushTexture);
		}
		SetBrushFromMaterial(MaterialToUse);
	}
}

void URHAsyncImage::SetBrushFromItemIcon(const UPlatformInventoryItem* Item, bool bMatchSize/* = false*/)
{
    if (Item)
    {
		if (UIconInfo* IconInfo = Item->GetItemIconInfo())
		{
			SetBrushFromIconInfo(IconInfo, bMatchSize);
		}
    }
    else if (MaterialToUse != nullptr && MaterialToUse == GetBrush().GetResourceObject())
    {
		CancelImageStreaming();

		MaterialToUse->SetTextureParameterValue(MaterialParameter, nullptr);

		if (bMatchSize)
		{
			FSlateBrush CurrentBrush = GetBrush();
			CurrentBrush.ImageSize = FVector2D(0, 0);
			SetBrush(CurrentBrush);
		}

		if (MyImage.IsValid())
		{
#if RH_FROM_ENGINE_VERSION(5,2)
			MyImage->InvalidateImage();
#else
			MyImage->SetImage(&Brush);
#endif
		}

		HideWaitingWidget();
		OnAsyncImageBrushChanged.Broadcast(this);
    }
	else
	{
		SetBrushFromTexture(nullptr, bMatchSize);
	}
}

void URHAsyncImage::SetBrushFromTextureOnItem(const UPlatformInventoryItem* Item, TSoftObjectPtr<UTexture2D> Texture, bool bMatchSize/* = false*/)
{
	TWeakObjectPtr<URHAsyncImage> WeakThis(this); // using weak ptr in case 'this' has gone out of scope by the time this lambda is called

	RequestAsyncLoad(Texture,
		[WeakThis, Texture, bMatchSize]() {
			if (URHAsyncImage* StrongThis = WeakThis.Get())
			{
				UTexture2D* StrongTexture = Texture.Get();
				ensureMsgf(StrongTexture, TEXT("Failed to load %s"), *Texture.ToSoftObjectPath().ToString());
				StrongThis->InternalSetBrushFromItemTexture(StrongTexture, bMatchSize);
			}
		});
}

void URHAsyncImage::SetBrushFromPathOnItem(const UPlatformInventoryItem* Item, const FSoftObjectPath& Path, bool bMatchSize /*= false*/)
{
	TWeakObjectPtr<URHAsyncImage> WeakThis(this); // using weak ptr in case 'this' has gone out of scope by the time this lambda is called
	TSoftObjectPtr<UObject> SoftObject(Path);

	RequestAsyncLoad(SoftObject,
		[WeakThis, SoftObject, bMatchSize]() {
		if (URHAsyncImage* StrongThis = WeakThis.Get())
		{
			UObject* StrongObject = SoftObject.Get();
			ensureMsgf(StrongObject, TEXT("Failed to load %s"), *SoftObject.ToSoftObjectPath().ToString());
			if(UTexture2D* StrongTexture = Cast<UTexture2D>(StrongObject))
			{
				StrongThis->InternalSetBrushFromItemTexture(StrongTexture, bMatchSize);
			}
			else if (UMaterialInterface* StrongMI = Cast<UMaterialInterface>(StrongObject))
			{
				StrongThis->SetBrushFromMaterial(StrongMI);
			}
			else
			{
				ensureMsgf(false, TEXT("Object %s is not a valid object type. Must be Texture2D or MaterialInterface."), *GetNameSafe(StrongObject));
				StrongThis->InternalSetBrushFromItemTexture(nullptr, bMatchSize);
			}
		}
	});
}