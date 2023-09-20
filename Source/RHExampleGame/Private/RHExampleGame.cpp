// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RHExampleGame.h"

#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FRHExampleGameModule, RHExampleGame, "RHExampleGame" );

DEFINE_LOG_CATEGORY(RHExampleGame);

void FRHExampleGameModule::StartupModule()
{
    UE_LOG(RHExampleGame, Log, TEXT("Game Module - StartupModule BEGIN"));

    UE_LOG(RHExampleGame, Log, TEXT("Game Module - StartupModule END"));
}

void FRHExampleGameModule::ShutdownModule()
{
    UE_LOG(RHExampleGame, Log, TEXT("Game Module - ShutdownModule BEGIN"));

    UE_LOG(RHExampleGame, Log, TEXT("Game Module - ShutdownModule END"));
}
