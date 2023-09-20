// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Widgets/RHWidget.h"
#include "PlatformMediaSource.h"
#include "Engine/StreamableManager.h"
#include "RHMediaPlayerWidget.generated.h"

USTRUCT(BlueprintType)
struct RALLYHERESTART_API FRHMediaPlayerWidgetPlaylistEntry : public FTableRowBase
{
	GENERATED_BODY()

public:
	// When true, movie can be skipped
	UPROPERTY(EditDefaultsOnly, Category = "Skipping behavior")
	bool bIsSkippable;

	// Number of seconds before skip option is present (entry must be skippable)
	UPROPERTY(EditDefaultsOnly, Category = "Skipping behavior")
	float SkippableAfter;

	// if true, this entry will never be skippable the first time it is presented
	UPROPERTY(EditDefaultsOnly, Category = "Skipping behavior")
	bool bForceFirstWatch;

	// if true, this entry will only play once, per version
	UPROPERTY(EditDefaultsOnly, Category = "Skipping behavior")
	bool bOnlyWatchOnce;

	// version number, works with all skippable options, resetting their behavior when the last-watched version is less than the current
	UPROPERTY(EditDefaultsOnly)
	uint16 MajorVersion;

	// Media Platform Source for playback
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UPlatformMediaSource> PlatformMediaSource;

	UPROPERTY(EditDefaultsOnly)
	FName LocalActionName;

	FRHMediaPlayerWidgetPlaylistEntry()
		: bIsSkippable(true)
		, SkippableAfter(0.f)
		, bForceFirstWatch(false)
		, bOnlyWatchOnce(false)
		, MajorVersion(1)
		, PlatformMediaSource(nullptr)
	{}
};

/**
 * RoCo UMG widget for playing Media Framework videos
 */
UCLASS(Config = Game)
class RALLYHERESTART_API URHMediaPlayerWidget : public URHWidget
{
	GENERATED_BODY()

	virtual void InitializeWidget_Implementation() override;
	virtual void ShowWidget() override;

	void CleanUp();

	// closes the widget and goes home
	void CloseMediaPlayerWidget();

// Playlist Entries management
public:
	// Hard reference to the DataTable to use. If not set, will attempt to use PlaylistDataTableClassName
	UPROPERTY(EditDefaultsOnly)
	UDataTable* PlaylistDataTable;

	// Path to the DataTable to load, configurable per game
	UPROPERTY(Config)
	FSoftObjectPath PlaylistDataTableClassName;

	TSharedPtr<FStreamableHandle> MediaAsyncLoadHandle;

	UPROPERTY(EditDefaultsOnly)
	bool bOnlyWatchFirstEntry;

	void CleanupAsyncLoadHandle();
	void PlayNextPlaylistEntry();
	void HandleMediaLoaded();
	// called at the beginning of the loading of the playlist entry datatable
	UFUNCTION(BlueprintImplementableEvent, Category = "Media Player Widget")
	void OnBeginLoadingMedia();
	// called at the end of the loading of the playlist entry datatable
	UFUNCTION(BlueprintImplementableEvent, Category = "Media Player Widget")
	void OnEndLoadingMedia();

	bool IsPlaying() const { return bPlaying; }

// Playback handling
public:
	// Called when a playlist entry is ready for playback
	UFUNCTION(BlueprintNativeEvent, Category = "Media Player Widget")
	void OnReadyForPlayback(const UPlatformMediaSource* PlatformMediaSource);

// Skipping handling
public:
	// updates the view to show or hide the prompt
	UFUNCTION(BlueprintImplementableEvent, Category = "Media Player Widget")
	void OnShouldShowPromptChanged(bool bCanSkipEntry);

	// accessor for whether the prompt should be shown and input should be process for skipping
	UFUNCTION(BlueprintPure, Category = "Media Player Widget")
	bool IsCurrentEntrySkippable() const { return bCurrentEntrySkippable;  }

	// user interaction callable function that triggers next entry in playlist (view should gate interaction based on IsCurrentEntrySkippable)
	UFUNCTION(BlueprintCallable, Category = "Media Player Widget")
	void UIX_SkipEntry();

private:
	FTimerHandle SkipPromptDelayTimerHandle;
	bool bCurrentEntrySkippable;
	bool bPlaying;
	int32 PlaylistEntriesWatched;
	FRHMediaPlayerWidgetPlaylistEntry* CurrentPlaylistEntry;

	void SetupSkipPrompting(bool BlockSkipping, float SkipDelay);
	void UpdateSkipPromptDelayTimer();
	void ClearUpdateSkipPromptDelayTimer();
	// setter for whether the prompt should be shown and input should be processed for skipping
	void SetCurrentEntrySkippable(bool bCanSkipEntry);
};