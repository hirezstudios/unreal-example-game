// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereEventClientModule.h"
#include "RH_EventClient.h"
#include "CoreGlobals.h"
#include "Misc/CoreDelegates.h"


DEFINE_LOG_CATEGORY(LogRallyHereEvent);

IMPLEMENT_MODULE(FRH_EventClientModule, RallyHereEventClient);

// global variable to allow reading of GConfig variables
FString GRH_EventClientIni;

void FRH_EventClientModule::StartupModule()
{
    FConfigCacheIni::LoadGlobalIniFile(GRH_EventClientIni, TEXT("RallyHereIntegration"));

    FRH_EventClient::InitSingleton();
}

void FRH_EventClientModule::ShutdownModule()
{
    FRH_EventClient::CleanupSingleton();
}