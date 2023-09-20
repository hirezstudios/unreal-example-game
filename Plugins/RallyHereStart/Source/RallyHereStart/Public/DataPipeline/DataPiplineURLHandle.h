// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

DECLARE_MULTICAST_DELEGATE(FDataPipelineURLIsReady);

extern RALLYHERESTART_API int32 GUseLegacyDataPiplineURLHandle;

enum class EDataPipelineURLState
{
    NotReady,
    Requesting,
    Ready,
    Unavailable,
};

class RALLYHERESTART_API FDataPipelineURLHandle : public TSharedFromThis<FDataPipelineURLHandle>
{
public:
    FDataPipelineURLHandle();
    virtual ~FDataPipelineURLHandle();
    virtual EDataPipelineURLState GetDataPipelineURLState() const { return CurrentState; }
    virtual FString GetDataPipelineURL() const { return FString(); }
    void Init();

    FDataPipelineURLIsReady DataPipelineURLCallback;

protected:

    EDataPipelineURLState CurrentState;
    virtual void InternalInit();
    void FinishURLIsReady();
};

class RALLYHERESTART_API  FDataPipelineURLHandle_Direct : public FDataPipelineURLHandle
{
public:
    typedef FDataPipelineURLHandle Super;

    FDataPipelineURLHandle_Direct(const FString& InURL);
    virtual ~FDataPipelineURLHandle_Direct();
    virtual FString GetDataPipelineURL() const override { return URL; }

protected:
    virtual void InternalInit() override;

private:
    FString URL;
};
