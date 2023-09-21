// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "Engine/StreamableManager.h"
#include "PlatformInventoryItem/PlatformInventoryItem.h"
#include "RH_Common.h"
#include "RHAsyncImage.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAsyncImageEvent, class URHAsyncImage*, Image);

class UIconInfo;

UCLASS(HideFunctions = (Appearance))
class RALLYHERESTART_API URHAsyncImage : public UImage
{
	GENERATED_BODY()
	
public:
	URHAsyncImage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintNativeEvent, Category = "Texture")
    void ShowWaitingWidget();

    UFUNCTION(BlueprintNativeEvent, Category = "Texture")
    void HideWaitingWidget();

    virtual void OnImageStreamingStarted(TSoftObjectPtr<UObject> SoftObject) override
    {
        Super::OnImageStreamingStarted(SoftObject);
        ShowWaitingWidget();
        OnAsyncImageLoadStarted.Broadcast(this);
    }

    virtual void OnImageStreamingComplete(TSoftObjectPtr<UObject> LoadedSoftObject) override
    {
        Super::OnImageStreamingComplete(LoadedSoftObject);
        HideWaitingWidget();
        OnAsyncImageLoadComplete.Broadcast(this);
    }
    
    virtual void CancelImageStreaming() override
    {
        if (StreamingHandle.IsValid())
        {
            Super::CancelImageStreaming();
            OnAsyncImageLoadCanceled.Broadcast(this);
            HideWaitingWidget();
        }
    }

    virtual void SetBrush(const FSlateBrush& InBrush) override
    {
        Super::SetBrush(InBrush);
        HideWaitingWidget();
        OnAsyncImageBrushChanged.Broadcast(this);
    }

	// this function is defined in 5.2, and direct access to Brush deprecated, this allows use of the new getter to allow easier coding
#if RH_BELOW_ENGINE_VERSION(5,2)
	const FSlateBrush& GetBrush() const
	{
		return Brush;
	}
#endif

    virtual void SetBrushFromAsset(USlateBrushAsset* Asset) override
    {
        Super::SetBrushFromAsset(Asset);
        HideWaitingWidget();
        OnAsyncImageBrushChanged.Broadcast(this);
    }

    virtual void SetBrushFromTexture(UTexture2D* Texture, bool bMatchSize = false) override
    {
        Super::SetBrushFromTexture(Texture, bMatchSize);
        HideWaitingWidget();
        OnAsyncImageBrushChanged.Broadcast(this);
    }

    virtual void SetBrushFromAtlasInterface(TScriptInterface<ISlateTextureAtlasInterface> AtlasRegion, bool bMatchSize = false) override
    {
        Super::SetBrushFromAtlasInterface(AtlasRegion, bMatchSize);
        HideWaitingWidget();
        OnAsyncImageBrushChanged.Broadcast(this);
    }

    virtual void SetBrushFromTextureDynamic(UTexture2DDynamic* Texture, bool bMatchSize = false) override
    {
        Super::SetBrushFromTextureDynamic(Texture, bMatchSize);
        HideWaitingWidget();
        OnAsyncImageBrushChanged.Broadcast(this);
    }

    virtual void SetBrushFromMaterial(UMaterialInterface* Material) override
    {
        Super::SetBrushFromMaterial(Material);
        HideWaitingWidget();
        OnAsyncImageBrushChanged.Broadcast(this);
    }

	UFUNCTION(BlueprintCallable, Category = "Texture", meta = (AutoCreateRefTerm = "SoftPath"))
	void SetBrushFromSoftPath(const FSoftObjectPath& SoftPath, bool bMatchSize);

	// Sets a brush from a specific icon from IconInfo
	UFUNCTION(BlueprintCallable, Category = "Texture")
	void SetBrushFromIconInfo(UIconInfo* IconInfo, bool bMatchSize = false);

	UFUNCTION(BlueprintCallable, Category = "Texture")
    void SetBrushFromItemIcon(const UPlatformInventoryItem* Item, bool bMatchSize = false);

    // Sets a brush from a specific icon from an item
    UFUNCTION(BlueprintCallable, Category = "Texture")
    void SetBrushFromTextureOnItem(const UPlatformInventoryItem* Item, TSoftObjectPtr<UTexture2D> Texture, bool bMatchSize = false);

	// Sets a brush from a specific icon from an item
	UFUNCTION(BlueprintCallable, Category = "Texture", meta=(AutoCreateRefTerm="Path"))
	void SetBrushFromPathOnItem(const UPlatformInventoryItem* Item, const FSoftObjectPath& Path, bool bMatchSize = false);

	UFUNCTION(BlueprintPure)
	bool IsCurrentlyAsyncLoading() const { return StreamingHandle.IsValid(); };

	protected:
	UFUNCTION(BlueprintSetter)
	void SetMaterialToUse(UMaterialInstanceDynamic* InMID);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waiting Widget")
	UWidget* WaitingWidget;

	void InternalSetBrushFromItemTexture(UTexture2D* StrongTexture, bool bMatchSize);

public:
	UFUNCTION(BlueprintCallable, Category="Waiting Widget")
	void SetWaitingWidget(UWidget* InWaitingWidget);

    UPROPERTY(BlueprintAssignable)
    FOnAsyncImageEvent OnAsyncImageLoadStarted;
    UPROPERTY(BlueprintAssignable)
    FOnAsyncImageEvent OnAsyncImageLoadComplete;
    UPROPERTY(BlueprintAssignable)
    FOnAsyncImageEvent OnAsyncImageLoadCanceled;
    UPROPERTY(BlueprintAssignable)
    FOnAsyncImageEvent OnAsyncImageBrushChanged;

	//get into a bad state. This needs some cleanup at a later date.
	UPROPERTY(Transient, BlueprintReadWrite, BlueprintSetter = SetMaterialToUse, Category = "Filter Material")
	UMaterialInstanceDynamic* MaterialToUse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter Material")
	FName MaterialParameter;

private:
	friend class UImageIconInfo;

	static const FName DefaultTextureParameterName;
};
