// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

class FRHExampleGameModule : public FDefaultGameModuleImpl
{
    /**
    * Called right after the module DLL has been loaded and the module object has been created
    */
    virtual void StartupModule() override;

    /**
    * Called right after the module DLL has been loaded and the module object has been created
    */
    virtual void ShutdownModule() override;
};

DECLARE_LOG_CATEGORY_EXTERN(RHExampleGame, Log, All);
