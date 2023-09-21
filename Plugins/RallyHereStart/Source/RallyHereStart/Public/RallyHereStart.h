// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UMG.h"

DECLARE_LOG_CATEGORY_EXTERN(RallyHereStart, Log, All);

class FRallyHereStartModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
