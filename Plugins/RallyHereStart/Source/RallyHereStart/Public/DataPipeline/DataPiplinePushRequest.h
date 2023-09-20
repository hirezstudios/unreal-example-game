// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Serialization/BitWriter.h"

class FDataPipelineURLHandle;

class IHttpRequest;
class IHttpResponse;

typedef TSharedPtr<class IHttpRequest> FHttpRequestPtr;
typedef TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> FHttpResponsePtr;

enum class EDataPipelinePushState : uint8
{
    NotStarted,
    Processing,
    Failed,
    Success,
};

class RALLYHERESTART_API FDataPipelinePushPayload : public TSharedFromThis<FDataPipelinePushPayload>
{
public:
    FDataPipelinePushPayload(const FString& InPayloadName, const FString& InContent);
    FDataPipelinePushPayload(const FString& InPayloadName, const TArray<uint8>& InContent);
    FDataPipelinePushPayload(const FString& InPayloadName, TArray<uint8>&& InContent);
    ~FDataPipelinePushPayload();
    FString GetPayloadName() const { return PayloadName; }
    const TArray<uint8>& GetBuffer() const { return Buffer; }
private:
    FString PayloadName;
    TArray<uint8> Buffer;
};



class RALLYHERESTART_API FDataPipelinePushRequest : public TSharedFromThis<FDataPipelinePushRequest>
{
public:
    FDataPipelinePushRequest(const TSharedPtr<FDataPipelineURLHandle>& InURLHandle, const TSharedPtr<FDataPipelinePushPayload>& InPayload);
    ~FDataPipelinePushRequest();
    void ProcessRequest();

private:
    void OnURLHandleReady();
    void BeginDataPipelinePush();
    void DataPiplineHttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Success);
    void HandleURLUnavailable();
    void FinishRequest();

    TSharedPtr<FDataPipelineURLHandle> URLHandle;
    TSharedPtr<FDataPipelinePushPayload> PayloadPtr;
    FHttpRequestPtr HttpRequestPtr;
    EDataPipelinePushState State;
};

class RALLYHERESTART_API FDataPipelinePushRequestManager
{
public:
    FDataPipelinePushRequestManager();
    ~FDataPipelinePushRequestManager();

    TSharedPtr<FDataPipelinePushRequest> CreateNewRequest(const TSharedPtr<FDataPipelineURLHandle>& InURLHandle, const TSharedPtr<FDataPipelinePushPayload>& InPayload);

    static FDataPipelinePushRequestManager* Get();

    static void StaticInit();
    static void StaticShutdown();

    void RequestFinished(const TSharedRef<FDataPipelinePushRequest>& InRequest);

private:
    static bool bIsAvailable;
    static FDataPipelinePushRequestManager* Singleton;

    TSet<TSharedPtr<FDataPipelinePushRequest>> ActivePipelinePushRequests;
};