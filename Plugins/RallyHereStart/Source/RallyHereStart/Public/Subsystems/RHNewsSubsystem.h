// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/RHProgression.h"
#include "RH_PlayerInfoSubsystem.h"
#include "Templates/SharedPointer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RHNewsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJsonPanelUpdated, const FString&, JsonName);

UCLASS(BlueprintType)
class RALLYHERESTART_API URHJsonData : public UObject
{
    GENERATED_BODY()

public:
	// Item Unique ID to be store after viewed
	UPROPERTY(BlueprintReadOnly)
	FName UniqueId;

	// The Loot ID that the JSON directly references
	UPROPERTY(BlueprintReadOnly)
	int32 AssociatedLootId;

    // Item ID to filter against for hiding
    UPROPERTY(BlueprintReadOnly)
    TArray<int32> HideIfItemOwned;

	// Item ID to filter against for showing
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ShowIfItemOwned;

	// Hide this if the associated item is already owned
	UPROPERTY(BlueprintReadOnly)
	bool HideIfOwned;

	// Player level gatekeeping, -1 if there's none
	UPROPERTY()
	int32 MinLevel;
	UPROPERTY()
	int32 MaxLevel;

	// Timestamps for a start-end window
	FDateTime StartTime;
	FDateTime EndTime;

	UPROPERTY(BlueprintReadOnly)
	bool showSteam;
	UPROPERTY(BlueprintReadOnly)
	bool showEpic;
	UPROPERTY(BlueprintReadOnly)
	bool showPS4;
	UPROPERTY(BlueprintReadOnly)
	bool showPS5;
	UPROPERTY(BlueprintReadOnly)
	bool showXB1;
	UPROPERTY(BlueprintReadOnly)
	bool showXSX;
	UPROPERTY(BlueprintReadOnly)
	bool showNX;
	UPROPERTY(BlueprintReadOnly)
	bool showIOS;
	UPROPERTY(BlueprintReadOnly)
	bool showAndroid;
};

USTRUCT(BlueprintType)
struct FRHJsonDataWrapper
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	TSet<URHJsonData*> JsonDataSet;
};

USTRUCT(BlueprintType)
struct FRHShouldShowPanelsWrapper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TMap<URHJsonData*, bool> ShouldShowByPanel;
};

DECLARE_DELEGATE_OneParam(FOnGetShouldShowPanels, FRHShouldShowPanelsWrapper);

UCLASS()
class URH_PlayerShouldShowPanelsHelper : public UObject
{
	GENERATED_BODY()

public:
	void StartCheck();

	int32 Responses;
	bool bComplete;

	FOnGetShouldShowPanels Event;
	TArray<URHJsonData*> PanelsToCheck;
	FRHShouldShowPanelsWrapper Results;

	UPROPERTY()
	URH_PlayerInfo* PlayerInfo;

	UPROPERTY()
	TWeakObjectPtr<URHNewsSubsystem> NewsSubsystem;

private:
	void OnGetShouldShowResponse(bool bShouldShow, URHJsonData* JsonData);
};

USTRUCT(BlueprintType)
struct FRHInventoryCountWrapper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TMap<FRH_ItemId, int32> InventoryCountsById;
};

DECLARE_DELEGATE_OneParam(FOnGetInventoryCounts, FRHInventoryCountWrapper);

UCLASS()
class URH_PlayerInventoryCountHelper : public UObject
{
	GENERATED_BODY()

public:
	void StartCheck();

	int32 Responses;
	bool bComplete;

	FOnGetInventoryCounts Event;
	TArray<FRH_ItemId> ItemIdsToCheck;
	FRHInventoryCountWrapper Results;

	UPROPERTY()
	const URH_PlayerInfo* PlayerInfo;

private:
	void OnGetCountResponse(int32 Count, FRH_ItemId ItemId);
};

DECLARE_DELEGATE_OneParam(FOnShouldShow, bool);

UCLASS(Config = game)
class RALLYHERESTART_API URHNewsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UTexture2DDynamic* GetTextureByRemoteURL(const TSharedPtr<FJsonObject>* ImageUrlJson);

    void LoadData(URHJsonData* JsonData, const TSharedPtr<FJsonObject>* JsonObject);

	void OnInventoryItemsUpdated(const TArray<int32>& UpdatedInventoryIds, URH_PlayerInfo* PlayerInfo);

    void CheckShouldShowForPlayer(URHJsonData* pData, URH_PlayerInfo* PlayerInfo, FOnShouldShow Delegate, bool bCheckPlatform = true);
	void CheckShouldShowForPlayer(TArray<URHJsonData*> pData, URH_PlayerInfo* PlayerInfo, FOnGetShouldShowPanels Delegate, bool bCheckPlatform = true);

    static FString GetLocalizedStringFromObject(const TSharedPtr<FJsonObject>* StringObject);

	static FName GetNameFromObject(const TSharedPtr<FJsonObject>* NameObject);

    TSharedPtr<FJsonObject> GetJsonPanelByName(const FString& name);

	UPROPERTY()
	TMap<FString, UTexture2DDynamic*> MapFilePathToTexture;

	UPROPERTY()
	TMap<FString, TWeakObjectPtr<UTexture2DDynamic>> FilePathToWeakTexture;

	TMap<FString, FString> MapRemoteUrlToFilePath;

	FJsonPanelUpdated JsonPanelUpdated;

protected:
    TMap<FString, TSharedPtr<FJsonObject>> JsonPanels;
	TMap<FString, TArray<int32>> ItemsPerPanel;

	UPROPERTY(BlueprintReadWrite)
	TMap<URH_PlayerInfo*, FRHJsonDataWrapper> CachedJsonDataByPlayer;

	UFUNCTION()
    void HandleJsonReady(class URHLandingPanelJSONHandler* pHandler);

	UFUNCTION()
    void HandleImagesReady(URHLandingPanelJSONHandler* pHandler);

    UTexture2DDynamic* LoadTexture(const uint8* contentData, int32 contentLength, FString strSaveFilePath);

private:
	void TryLoadLandingPanels();

	UPROPERTY(config)
	FSoftObjectPath PlayerProgressionXpClass;

	UPROPERTY()
	TSoftObjectPtr<URHProgression> PlayerXpProgression;
};