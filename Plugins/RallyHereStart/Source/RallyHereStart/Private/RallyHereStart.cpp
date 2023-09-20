// Copyright 2016-2017 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "DataFactories/RHSettingsDataFactory.h"
#include "DataPipeline/DataPiplinePushRequest.h"
#include "EventClient/RallyHereEventClientIntegration.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

IMPLEMENT_GAME_MODULE(FRallyHereStartModule, RallyHereStart);

DEFINE_LOG_CATEGORY(RallyHereStart);

#define LOCTEXT_NAMESPACE "RHUI"

void FRallyHereStartModule::StartupModule()
{
	UE_LOG(RallyHereStart, Log, TEXT("RallyHere Start Module - StartupModule BEGIN"));

#if WITH_EDITOR
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->RegisterSettings("Project", "Game", "RHSettingsDataFactory",
            LOCTEXT("RHUINewSettingsName", "Settings Page"),
            LOCTEXT("RHUINewSettingsDescription", "Configure Player Settings"),
            GetMutableDefault<URHSettingsDataFactory>()
        );
    }
#endif

	FRallyHereEventClientIntegration::InitSingleton();
	FDataPipelinePushRequestManager::StaticInit();

	UE_LOG(RallyHereStart, Log, TEXT("RallyHere Start Module - StartupModule END"));
}

void FRallyHereStartModule::ShutdownModule()
{
	UE_LOG(RallyHereStart, Log, TEXT("RallyHere Start Module - ShutdownModule BEGIN"));

#if WITH_EDITOR
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Game", "RHSettingsDataFactory");
    }
#endif

	FRallyHereEventClientIntegration::CleanupSingleton();
	FDataPipelinePushRequestManager::StaticShutdown();

	UE_LOG(RallyHereStart, Log, TEXT("RallyHere Start Module - ShutdownModule END"));
}

#undef LOCTEXT_NAMESPACE
