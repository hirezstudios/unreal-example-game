// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Engine/AssetManager.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Lobby/Widgets/RHMediaPlayerWidget.h"
#include "GameFramework/RHGameUserSettings.h"
#include "MediaPlayer.h"
#include "MediaPlayerFacade.h"

void URHMediaPlayerWidget::InitializeWidget_Implementation()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::InitializeWidget_Implementation()"));
	
	if (!PlaylistDataTable)
	{
		if (PlaylistDataTableClassName.ToString().Len() > 0)
		{
			PlaylistDataTable = LoadObject<UDataTable>(nullptr, *PlaylistDataTableClassName.ToString(), nullptr, LOAD_None, nullptr);
		}
	}
	
	CleanUp();

	Super::InitializeWidget_Implementation();
}

void URHMediaPlayerWidget::ShowWidget()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::ShowWidget()"));
	CleanUp();

	// queue the next entry up
	PlayNextPlaylistEntry();

	Super::ShowWidget();
}

void URHMediaPlayerWidget::CleanUp()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::CleanUp()"));
	CleanupAsyncLoadHandle();
	ClearUpdateSkipPromptDelayTimer();
	SetCurrentEntrySkippable(false);
	bPlaying = false;
	PlaylistEntriesWatched = 0;
}

void URHMediaPlayerWidget::CloseMediaPlayerWidget()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::CloseMediaPlayerWidget()"));
	RemoveViewRoute(MyRouteName);
	CleanUp();
}

void URHMediaPlayerWidget::CleanupAsyncLoadHandle()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::CleanupAsyncLoadHandle()"));
	if (MediaAsyncLoadHandle.IsValid())
	{
		MediaAsyncLoadHandle->CancelHandle();
	}
	
	MediaAsyncLoadHandle.Reset();
}

void URHMediaPlayerWidget::PlayNextPlaylistEntry()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::PlayNextPlaylistEntry()"));

	// clean up any skip prompt handling
	ClearUpdateSkipPromptDelayTimer();
	SetCurrentEntrySkippable(false);

	CleanupAsyncLoadHandle();

	if (!bOnlyWatchFirstEntry || PlaylistEntriesWatched == 0)
	{
		if (PlaylistDataTable)
		{
			bPlaying = true;

			// queue up the first media player entry for playback
			TArray<FName> RowNames = PlaylistDataTable->GetRowNames();
			int PlaylistSkipCount = PlaylistEntriesWatched;
			for (auto& RowName : RowNames)
			{
				if (PlaylistSkipCount > 0)
				{
					PlaylistSkipCount--;
					continue;
				}
				
				CurrentPlaylistEntry = PlaylistDataTable->FindRow<FRHMediaPlayerWidgetPlaylistEntry>(RowName, "Get playList entry.");
				if (!CurrentPlaylistEntry)
				{
					continue;
				}

				bool bHaveWatchedBefore = false;
				if (!CurrentPlaylistEntry->LocalActionName.IsNone())
				{
					if (URHGameUserSettings* GameSettings = Cast<URHGameUserSettings>(GEngine->GetGameUserSettings()))
					{
						bHaveWatchedBefore = GameSettings->IsLocalActionSaved(CurrentPlaylistEntry->LocalActionName);

						if (!bHaveWatchedBefore)
						{
							GameSettings->SaveLocalAction(CurrentPlaylistEntry->LocalActionName);
						}
					}
				}

				bool bBlockSkipping = false;
				if (bHaveWatchedBefore && CurrentPlaylistEntry->bOnlyWatchOnce)
				{
					// skip it, we've seen it and it's marked only to be watched once
					continue;
				}
				else
				{
					// should we block skipping on the first watch?
					bBlockSkipping = (CurrentPlaylistEntry->bForceFirstWatch && !bHaveWatchedBefore) ? true : !CurrentPlaylistEntry->bIsSkippable;
				}
				
				SetupSkipPrompting(bBlockSkipping, CurrentPlaylistEntry->SkippableAfter);

				UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::PlayNextPlaylistEntry() - passing up the next entry for playback to the view layer."));
				PlaylistEntriesWatched++;

				// update view layer with loading state
				OnBeginLoadingMedia();

				// load the media entry assets from the softobj pointers
				TArray<FSoftObjectPath> MediaPaths = { CurrentPlaylistEntry->PlatformMediaSource.ToSoftObjectPath() };
				struct FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
				MediaAsyncLoadHandle = StreamableManager.RequestAsyncLoad(MediaPaths, FStreamableDelegate::CreateUObject(this, &URHMediaPlayerWidget::HandleMediaLoaded));
				return;
			}
			
			// if we get here, we've watched all of the entries
			UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::PlayNextPlaylistEntry() - No more entries to watch."));
		}
		else
		{
			// problem loading playlist entries
			UE_LOG(RallyHereStart, Warning, TEXT("URHMediaPlayerWidget::PlayNextPlaylistEntry() - Problem loading playlist entries."));
		}
	}

	CloseMediaPlayerWidget();
}

void URHMediaPlayerWidget::HandleMediaLoaded()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::HandleMediaLoaded()"));

	OnEndLoadingMedia();
	
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::HandleMediaLoaded() %p %i %i %i"), CurrentPlaylistEntry, MediaAsyncLoadHandle.IsValid(), MediaAsyncLoadHandle->HasLoadCompleted(), CurrentPlaylistEntry ? (int32)CurrentPlaylistEntry->PlatformMediaSource.IsValid() : -1);
	if (CurrentPlaylistEntry && MediaAsyncLoadHandle.IsValid() && MediaAsyncLoadHandle->HasLoadCompleted() && CurrentPlaylistEntry->PlatformMediaSource.IsValid())
	{
		UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::HandleMediaLoaded() - passing up the next entry for playback to the view layer."));
		OnReadyForPlayback(CurrentPlaylistEntry->PlatformMediaSource.Get());
	}
}

void URHMediaPlayerWidget::OnReadyForPlayback_Implementation(const UPlatformMediaSource* PlatformMediaSource)
{
}

void URHMediaPlayerWidget::SetupSkipPrompting(bool BlockSkipping, float SkipDelay)
{
	if (BlockSkipping)
	{
		UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::SetupSkipPrompting() - skipping blocked."));
		return;
	}

	if (SkipDelay <= 0)
	{
		UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::SetupSkipPrompting() - immediately skippable."));
		SetCurrentEntrySkippable(true);
	}

	// kick off timer for the delayed prompt
	if (MyHud != nullptr)
	{
		UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::SetupSkipPrompting() - setting up skipping, delayed by %f seconds."), SkipDelay);
		MyHud->GetWorldTimerManager().SetTimer(
			SkipPromptDelayTimerHandle,
			this, &URHMediaPlayerWidget::UpdateSkipPromptDelayTimer,
			SkipDelay
		);
	}
}

void URHMediaPlayerWidget::UpdateSkipPromptDelayTimer()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::UpdateSkipPromptDelayTimer()"));
	ClearUpdateSkipPromptDelayTimer();
	SetCurrentEntrySkippable(true);
}

void URHMediaPlayerWidget::ClearUpdateSkipPromptDelayTimer()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::ClearUpdateSkipPromptDelayTimer()"));
	if (MyHud != nullptr && SkipPromptDelayTimerHandle.IsValid())
	{
		MyHud->GetWorldTimerManager().ClearTimer(SkipPromptDelayTimerHandle);
		SkipPromptDelayTimerHandle.Invalidate();
	}
}

void URHMediaPlayerWidget::SetCurrentEntrySkippable(bool bCanSkipEntry)
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::SetCurrentEntrySkippable(%s)"), (bCanSkipEntry ? TEXT("TRUE") : TEXT("FALSE")));
	if (bCanSkipEntry != bCurrentEntrySkippable)
	{
		bCurrentEntrySkippable = bCanSkipEntry;
		OnShouldShowPromptChanged(bCurrentEntrySkippable);
	}
}

void URHMediaPlayerWidget::UIX_SkipEntry()
{
	UE_LOG(RallyHereStart, Verbose, TEXT("URHMediaPlayerWidget::UIX_SkipEntry()"));
	PlayNextPlaylistEntry();
}
