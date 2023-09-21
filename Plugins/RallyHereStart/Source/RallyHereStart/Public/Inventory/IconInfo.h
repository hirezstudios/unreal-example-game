// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IconInfo.generated.h"

class UTexture2D;
class URHAsyncImage;

/** 
 * IconInfo is an object that contains information about about how to set up an icon.
 * At the moment it only uses Texture2D and Materials as icons, but in the 
 * future, we'd like the ability to use other resources like a custom UserWidget.
 * This object gives us the ability to easily switch that out if desired.
 */
UCLASS(Abstract, DefaultToInstanced, HideCategories = Object, EditInlineNew, BlueprintType)
class RALLYHERESTART_API UIconInfo : public UObject
{
	GENERATED_BODY()

public:
	UIconInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual TSoftObjectPtr<UTexture2D> GetSoftItemIcon() const { return TSoftObjectPtr<UTexture2D>(); }	
	virtual bool IsValidIcon() const { return false; }

	virtual void ApplyToAsyncImage(URHAsyncImage* InImage, bool bMatchSize = false) {}
};

UCLASS(AutoExpandCategories=Image)
class RALLYHERESTART_API UImageIconInfo : public UIconInfo
{
	GENERATED_BODY()

public:
	UImageIconInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Image", meta=(AssetBundles="Client,Icon", AllowedClasses="/Script/Engine.Texture2D,/Script/Engine.MaterialInterface"))
	FSoftObjectPath IconImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Image")
	FLinearColor IconTint;

	virtual bool IsValidIcon() const override { return !IconImage.IsNull(); }

	virtual void ApplyToAsyncImage(URHAsyncImage* InImage, bool bMatchSize = false) override;
};

FORCEINLINE TSoftObjectPtr<UTexture2D> GetIconTextureSafe(const UIconInfo* IconInfo)
{
	return IconInfo != nullptr ? IconInfo->GetSoftItemIcon() : TSoftObjectPtr<UTexture2D>();
}