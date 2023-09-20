// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.
#include "DataPipeline/DataPiplineURLHandle.h"

#include "HttpModule.h"
#include "HttpManager.h"
#include "Interfaces/IHttpResponse.h"

#include "Containers/StringConv.h"
#include "Dom/JsonObject.h"

DEFINE_LOG_CATEGORY_STATIC(LogDataPipelineURL, Log, All);

FDataPipelineURLHandle::FDataPipelineURLHandle()
    : DataPipelineURLCallback()
    , CurrentState(EDataPipelineURLState::NotReady)
{
}

FDataPipelineURLHandle::~FDataPipelineURLHandle()
{
}

void FDataPipelineURLHandle::Init()
{
    if (CurrentState == EDataPipelineURLState::NotReady)
    {
        CurrentState = EDataPipelineURLState::Requesting;
        InternalInit();
    }
}

void FDataPipelineURLHandle::InternalInit()
{
    if (CurrentState != EDataPipelineURLState::Requesting)
    {
        return;
    }

    CurrentState = EDataPipelineURLState::Unavailable;
    FinishURLIsReady();
}

void FDataPipelineURLHandle::FinishURLIsReady()
{
    DataPipelineURLCallback.Broadcast();
    DataPipelineURLCallback.Clear();
}

FDataPipelineURLHandle_Direct::FDataPipelineURLHandle_Direct(const FString& InURL)
    : Super()
    , URL(InURL)
{
}

FDataPipelineURLHandle_Direct::~FDataPipelineURLHandle_Direct()
{
}

void FDataPipelineURLHandle_Direct::InternalInit()
{
    if (CurrentState != EDataPipelineURLState::Requesting)
    {
        return;
    }

    CurrentState = EDataPipelineURLState::Ready;
    FinishURLIsReady();
}
