// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRallyHereEvent, Log, All);

extern FString GRH_EventClientIni;

class FRH_EventClientModule : public IModuleInterface
{
public:
    FRH_EventClientModule() {}
    virtual ~FRH_EventClientModule() {}

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};