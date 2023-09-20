#include "EventClient/RallyHereEventClientIntegration.h"
#include "RallyHereStart.h"
#include "RH_EventClient.h"
#include "RH_LocalPlayerSubsystem.h"
#include "RH_GameInstanceSubsystem.h"
#include "CoreMinimal.h"
#include "Internationalization/Regex.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"

#include <string.h>

DEFINE_LOG_CATEGORY(LogRallyHereEventClientIntegration);

//------------------------------------------------
//
static bool IsMobileClient()
{
	const FString PlatformName = UGameplayStatics::GetPlatformName();
	if (PlatformName == TEXT("IOS") || PlatformName == TEXT("Android"))
	{
		return true;
	}
	return false;
}

TSharedPtr<FRallyHereEventClientIntegration> FRallyHereEventClientIntegration::Instance;
URHGameInstance* FRallyHereEventClientIntegration::GameInstance;

//------------------------------------------------
//
void FRallyHereEventClientIntegration::InitSingleton()
{
	// make an instance of me
	if (!Instance.IsValid())
	{
		UE_LOG(LogRallyHereEventClientIntegration, Log, TEXT("FRallyHereEventClientIntegration::InitSingleton creating instance"));
		Instance = TSharedPtr<FRallyHereEventClientIntegration>(new FRallyHereEventClientIntegration());
	}

	// make sure that the HiRezEvent(Http)Client is loaded
	TSharedPtr<FRH_EventClient> RallyHereEventClient = FRH_EventClient::Get();
    if (!RallyHereEventClient.IsValid())
    {
		UE_LOG(LogRallyHereEventClientIntegration, Log, TEXT("FRallyHereEventClientIntegration::InitSingleton could not load RallyHereEventClient"));
        return;
    }

	if (IsRunningDedicatedServer())
	{
		RallyHereEventClient->SetServerStarted();
	}

	if (IsRunningClientOnly() || IsRunningGame()) // any sort of client
	{
		FCoreDelegates::PostRenderingThreadCreated.AddLambda([]()
		{
			OnPostRenderingThreadCreated();
		});
	}
}

void FRallyHereEventClientIntegration::SetGameInstance(URHGameInstance* InGameInstance)
{
	GameInstance = InGameInstance;

	if (GameInstance != nullptr)
	{
		if (URH_GameInstanceSubsystem* pGISS = GameInstance->GetSubsystem<URH_GameInstanceSubsystem>())
		{
			if (URH_ConfigSubsystem* ConfigSubsystem = pGISS->GetConfigSubsystem())
			{
				GrabAppSettings(ConfigSubsystem);
				ConfigSubsystem->OnSettingsUpdated.AddStatic(&FRallyHereEventClientIntegration::GrabAppSettings);
			}
		}
	}
}

//------------------------------------------------
//
void FRallyHereEventClientIntegration::CleanupSingleton()
{
	if (Instance.IsValid())
	{
		UE_LOG(LogRallyHereEventClientIntegration, Log, TEXT("FRallyHereEventClientIntegration::CleanupSingleton"));
		Instance = nullptr;
	}
}

//------------------------------------------------
//
void FRallyHereEventClientIntegration::OnPostRenderingThreadCreated()
{
	int32 ResolutionX = 0;
	int32 ResolutionY = 0;

	// Now see what we can get from from GameUserSettings
	// Load the data
	FString LocalGameUserSettingsIni;
	FConfigCacheIni::LoadGlobalIniFile(LocalGameUserSettingsIni, TEXT("GameUserSettings"));

	// Grab the resolution
	FString SavedScreenResolution;
	if (GConfig->GetString(TEXT("/Script/RallyHereStart.RHGameUserSettings"), TEXT("SavedScreenResolution"), SavedScreenResolution, LocalGameUserSettingsIni))
	{
		// have to parse the thing: SavedScreenResolution=(X=1920,Y=1080)
		const int32 RezCaptureGroup = 1; // capture group 0 is the whole string

		FRegexPattern XFinder(TEXT("X=([0-9]+)"));
		FRegexMatcher XMatcher(XFinder, *SavedScreenResolution);
		if (XMatcher.FindNext())
		{
			int32 start = XMatcher.GetCaptureGroupBeginning(RezCaptureGroup);
			ResolutionX = TCString<TCHAR>::Atoi(*SavedScreenResolution + start);
		}

		FRegexPattern YFinder(TEXT("Y=([0-9]+)"));
		FRegexMatcher YMatcher(YFinder, *SavedScreenResolution);
		if (YMatcher.FindNext()) // get next
		{
			int32 start = YMatcher.GetCaptureGroupBeginning(RezCaptureGroup);
			ResolutionY = TCString<TCHAR>::Atoi(*SavedScreenResolution + start);
		}

		if (ResolutionX && ResolutionY)
		{	// let the plugin know this might be a better or changed value
			FRH_EventClientInterface::ChecksumCheckAndSaveScreenResolution(ResolutionX, ResolutionY);
		}
	}
}

void FRallyHereEventClientIntegration::OnPLayerLoginStatusChanged(ULocalPlayer* Player)
{
	if (URH_LocalPlayerSubsystem* RHSS = Player->GetSubsystem<URH_LocalPlayerSubsystem>())
	{
		if (RHSS->IsLoggedIn())
		{
			FRH_EventClientInterface::SendLoggedIn(RHSS->GetPlayerUuid().ToString());
		}
		else
		{
			FRH_EventClientInterface::SendDisconnected(RHSS->GetPlayerUuid().ToString());
		}
	}
}

void FRallyHereEventClientIntegration::OnRHLoginResponse(const FRH_LoginResult& LR)
{
	if (LR.Result != ERHAPI_LoginResult::Success)
	{
		FRH_EventClientInterface::SendLoginFailed(LR.OSSErrorMessage);
	}
}

//------------------------------------------------
//
void FRallyHereEventClientIntegration::OnLoginEvent_LoginRequested()
{
	FRH_EventClientInterface::SendLoginRequested();
}

void FRallyHereEventClientIntegration::GrabAppSettings(URH_ConfigSubsystem* ConfigSubsystem)
{
	TSharedPtr<FRH_EventClient> EventClient = FRH_EventClient::Get();
	
	FString SettingString;
	
	if (ConfigSubsystem->GetAppSetting("EventTracking.try_count", SettingString))
	{
		EventClient->SetTryCount(FCString::Atoi(*SettingString));
	}

	if (ConfigSubsystem->GetAppSetting("EventTracking.retry_interval_seconds", SettingString))
	{
		EventClient->SetRetryIntervalSeconds(FCString::Atoi(*SettingString));
	}

	if (ConfigSubsystem->GetAppSetting("EventTracking.bad_state_retry_interval_seconds", SettingString))
	{
		EventClient->SetBadStateRetryIntervalSeconds(FCString::Atoi(*SettingString));
	}

	if (ConfigSubsystem->GetAppSetting("EventTracking.send_bulk", SettingString))
	{
		EventClient->SetSendBulkEvents(SettingString == "1" || SettingString == "true");
	}

	if (ConfigSubsystem->GetAppSetting("EventTracking.bulk_send_threshold_count", SettingString))
	{
		EventClient->SetBulkSendThresholdCount(FCString::Atoi(*SettingString));
	}

	if (ConfigSubsystem->GetAppSetting("EventTracking.timed_send_threshold_seconds", SettingString))
	{
		EventClient->SetTimedSendThresholdSeconds(FCString::Atoi(*SettingString));
	}

	if (ConfigSubsystem->GetAppSetting("EventTracking.game_running_keepalive_seconds", SettingString))
	{
		EventClient->SetGameRunningKeepaliveSeconds(FCString::Atoi(*SettingString));
	}

	if (ConfigSubsystem->GetAppSetting("EventTracking.auto_pause_interval_seconds", SettingString))
	{
		EventClient->SetAutoPauseIntervalSeconds(FCString::Atoi(*SettingString));
	}

	if (IsMobileClient()) // check for override
	{
		if (ConfigSubsystem->GetAppSetting("EventTracking_Mobile.event_url", SettingString))
		{
			EventClient->SetEventURL(SettingString);
		}

		if (ConfigSubsystem->GetAppSetting("EventTracking_Mobile.project_id", SettingString))
		{
			EventClient->SetProjectId(SettingString);
		}

		if (ConfigSubsystem->GetAppSetting("EventTracking_Mobile.game_id", SettingString))
		{
			EventClient->SetGameId(FCString::Atoi(*SettingString));
		}

		if (ConfigSubsystem->GetAppSetting("EventTracking_Mobile.enable", SettingString))
		{
			EventClient->SetEnableEvents(SettingString == "1" || SettingString == "true");
		}
	}
	else
	{
		if (ConfigSubsystem->GetAppSetting("EventTracking.event_url", SettingString))
		{
			EventClient->SetEventURL(SettingString);
		}

		if (ConfigSubsystem->GetAppSetting("EventTracking.project_id", SettingString))
		{
			EventClient->SetProjectId(SettingString);
		}

		if (ConfigSubsystem->GetAppSetting("EventTracking.game_id", SettingString))
		{
			EventClient->SetGameId(FCString::Atoi(*SettingString));
		}

		if (ConfigSubsystem->GetAppSetting("EventTracking.enable", SettingString))
		{
			EventClient->SetEnableEvents(SettingString == "1" || SettingString == "true");
		}
	}
}