// Copyright 2016-2018 Hi-Rez Studios, Inc. All Rights Reserved.

#include "Engine/GameViewportClient.h"
#include "RallyHereEventClientModule.h"
#include "RH_EventClient.h"
#include "HttpModule.h"
#include "HttpManager.h"
#include "Interfaces/IHttpResponse.h"
#include "Internationalization/Culture.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "HAL/RunnableThread.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CString.h"

// -----------------------------------------------------------------------------------
// Module local helper classes.
// -----------------------------------------------------------------------------------

//------------------------------------------------
// ensure proper format
FORCEINLINE FString FRH_EventStandardGUID(const FGuid& Guid)
{
    return Guid.ToString(EGuidFormats::DigitsWithHyphens);
}

//------------------------------------------------
// ensure proper format
FORCEINLINE FString FRH_EventStandardGUID(FString StrGuid)
{
    FGuid StdGuid;
    StdGuid.Parse(StrGuid, StdGuid);
    return FRH_EventStandardGUID(StdGuid);
}

//------------------------------------------------
// make a new one
FORCEINLINE FString FRH_EventStandardGUID()
{
    return FRH_EventStandardGUID(FGuid::NewGuid());
}

//------------------------------------------------
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
//  algorithm fnv - 1a is
//      hash : = FNV_offset_basis
//
//      for each byte_of_data to be hashed do
//          hash : = hash XOR byte_of_data
//          hash : = hash x FNV_prime
//
//      return hash
//
// The FNV_offset_basis is the 64-bit FNV offset basis value: 14695981039346656037 (in hex, 0xcbf29ce484222325).
// The FNV_prime is the 64 - bit FNV prime value : 1099511628211 (in hex, 0x100000001b3).
// The XOR is an 8-bit operation that modifies only the lower 8-bits of the hash value.
// The FNV-1a hash differs from the FNV-1 hash by only the order in which the multiply and XOR is performed:
// The above pseudocode has the same assumptions that were noted for the FNV-1 pseudocode. The change in order leads to slightly better avalanche characteristics

//------------------------------------------------
//
template<typename T>
uint64 RH_Fowler_Noll_Vo(uint64 Acc, T Input) //example(64 - bit FNV - 1a)
{
    uint64 lowerBits = 0x00000000000000FF;
    uint64 upperBits = ~lowerBits;
    uint64 hash = Acc ? Acc : 0xcbf29ce484222325; // FNV_offset_basis or the passed in continuation of the hash
    for(int32 i = 0; i<sizeof(Input); i++)
    {
        hash = (hash & upperBits) | ((hash ^ Input) & lowerBits);
        hash = hash * 0x100000001b3; // FNV_prime
        Input >>= 8;
    }
    return hash;
}


//------------------------------------------------
//
uint64 RH_Fowler_Noll_Vo(const FString& Input)
{
    uint64 Acc = 0;
    for (auto Character : Input)
    {
        Acc = RH_Fowler_Noll_Vo(Acc, Character);
    }
    return Acc;
}

//------------------------------------------------
//
FORCEINLINE FGuid RH_StringChecksum(const FString& Input)
{
    uint64 FNVo = RH_Fowler_Noll_Vo(Input);
    FGuid Result(
        (uint32) ((FNVo >>32) & 0xFFFFFFFF)
        , (uint32) ((FNVo >> 0) & 0xFFFFFFFF)
        , 0
        , 0
    );
    return Result;
}

//------------------------------------------------
//
uint64 RH_Fowler_Noll_Vo(const FGuid& Input)
{
    uint64 Acc = 0;
    Acc = RH_Fowler_Noll_Vo(Acc, Input.A);
    Acc = RH_Fowler_Noll_Vo(Acc, Input.B);
    Acc = RH_Fowler_Noll_Vo(Acc, Input.C);
    Acc = RH_Fowler_Noll_Vo(Acc, Input.D);
    return Acc;
}


//------------------------------------------------
//
uint64 RH_Fowler_Noll_Vo(uint32 Input1, uint32 Input2=0, uint32 Input3=0)
{
    uint64 Acc = 0;
    Acc = RH_Fowler_Noll_Vo(Acc, Input1);
    Acc = RH_Fowler_Noll_Vo(Acc, Input2);
    Acc = RH_Fowler_Noll_Vo(Acc, Input3);
    return Acc;
}


//------------------------------------------------
//
FORCEINLINE FGuid RH_DataChecksum(const FGuid& Guid1, const FGuid& Guid2, const FGuid& Guid3)
{
    FGuid Result(
        //(uint32)Fowler_Noll_Vo(Guid1.A, Guid1.B) // GUID1 C,D are the RezXY values; they are expected to change so ignore them in the checksum
        (uint32)RH_Fowler_Noll_Vo(Guid1) // GUID1 C,D are the RezXY values; they are expected to change so we'll update the checksum when it does
        , (uint32)RH_Fowler_Noll_Vo(Guid2)
        , (uint32)RH_Fowler_Noll_Vo(Guid3)
        , 0
    );
    // D is the checksum of the other checksums, so self check
    Result.D = RH_Fowler_Noll_Vo(Result.A, Result.B, Result.C);
    return Result;
}


//------------------------------------------------
//
FRH_EventSimpleTimer::FRH_EventSimpleTimer(double _Duration)
{
    SetTimer(_Duration);
    ReStart();
}

//------------------------------------------------
//
void FRH_EventSimpleTimer::SetTimer(double _Duration)
{
    Duration = _Duration;
}

//------------------------------------------------
//
void FRH_EventSimpleTimer::ReStart()
{
    StartTime = FPlatformTime::Seconds();
    EndTime = StartTime + Duration;
}

//------------------------------------------------
//
double FRH_EventSimpleTimer::RemainingTime()
{
    return FPlatformTime::Seconds() - StartTime;
}

//------------------------------------------------
//
bool FRH_EventSimpleTimer::TimeExpired()
{
    return FPlatformTime::Seconds() >= EndTime;
}


// -----------------------------------------------------------------------------------
// Implementation of Generalized Event Tracking classes.
// -----------------------------------------------------------------------------------

//------------------------------------------------
//
void FRH_EventBase::SetEventName(const FString& _eventName)
{
    SetStringField(TEXT("eventName"), _eventName);
}


//------------------------------------------------
//
void FRH_EventBase::SetEventUUID()
{
    FString newGuidString = FRH_EventStandardGUID();
    JsonEvent->SetStringField(TEXT("eventUUID"), newGuidString);
}



//------------------------------------------------
// Fill in all the properties that are not influenced by any configuration
FRH_EventBase::FRH_EventBase(const FString& _EventName, const bool _SetSenderEventTimestamp)
    : EventName(_EventName)
{
    SetEventName(EventName);

    // always generate a unique event id for deduplication
    SetEventUUID();

    // We will choose event creation on the assumption we are getting set up to send right away.
    // If we want send time, we can call this again to update the value
    // We assume serialize is a transactional action of Send()
    if (_SetSenderEventTimestamp)
    {
        SetSenderEventTimestamp();
    }

    // go ahead and hook these up (because reference and not copy; cleanup managed by reference counting)
    JsonEvent->SetObjectField(TEXT("eventParams"), JsonParameters);
}


//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetStringField(const FString& FieldName, const FString& FieldValue)
{
    JsonEvent->SetStringField(FieldName, FieldValue);
    return *this;
}

//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetIntegerField(const FString& FieldName, const int32 FieldValue)
{
    JsonEvent->SetNumberField(FieldName, double(FieldValue));
    return *this;
}

//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetNumberField(const FString& FieldName, const double FieldValue)
{
    JsonEvent->SetNumberField(FieldName, FieldValue);
    return *this;
}

//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetParameterStringField(const FString& FieldName, const FString& FieldValue)
{
    JsonParameters->SetStringField(FieldName, FieldValue);
    return *this;
}

//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetParameterIntegerField(const FString& FieldName, const int32 FieldValue)
{
    JsonParameters->SetNumberField(FieldName, double(FieldValue));
    return *this;
}

//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetParameterBoolField(const FString& FieldName, const bool FieldValue)
{
    JsonParameters->SetBoolField(FieldName, FieldValue);
    return *this;
}

//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetParameterNumberField(const FString& FieldName, const double FieldValue)
{
    JsonParameters->SetNumberField(FieldName, FieldValue);
    return *this;
}

FRH_EventBase& FRH_EventBase::SetParameters(const FRH_JsonDataSet& Parameters)
{
    for (const auto& Parameter : Parameters)
    {
        switch (Parameter.GetType())
        {
        case FRH_JsonValue::ValueType_Integer:
            JsonParameters->SetNumberField(Parameter.GetName(), Parameter.IntegerValue);
            break;
        case FRH_JsonValue::ValueType_Double:
            JsonParameters->SetNumberField(Parameter.GetName(), Parameter.DoubleValue);
            break;
        case FRH_JsonValue::ValueType_Bool:
            JsonParameters->SetBoolField(Parameter.GetName(), Parameter.BoolValue);
            break;

        default:
        case FRH_JsonValue::ValueType_Guid:
        case FRH_JsonValue::ValueType_DateTime:
        case FRH_JsonValue::ValueType_String:
            JsonParameters->SetStringField(Parameter.GetName(), Parameter.StringValue);
            break;
        }
    }

    return *this;
}


//------------------------------------------------
//
FRH_EventBase& FRH_EventBase::SetSenderEventTimestamp()
{
    FDateTime UtcNow = FDateTime::UtcNow();
    FString TimestampString = UtcNow.ToIso8601().Replace(TEXT("T"), TEXT(" ")).Replace(TEXT("Z"), TEXT(""));
    // may need to strip out the 'T' and 'Z'
    SetStringField(TEXT("eventTimestamp"), TimestampString);
    return *this;
}


//------------------------------------------------
//
const FString& FRH_EventBase::SerializeToJson()
{
    // seems like we have to use TCHAR and NOT CHAR8 because we are serializing an FString
#if UE_BUILD_SHIPPING
    TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&SerializedJson);
#else
    TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&SerializedJson);
#endif
    FJsonSerializer::Serialize(JsonEvent, JsonWriter);
    return SerializedJson;
}


//------------------------------------------------
//
FRH_EventContainer::FRH_EventContainer(int32 _MaximumEventsToSend, int32 _MaximumBytesToSend)
    : MaximumEventsToSend(_MaximumEventsToSend)
    , MaximumBytesToSend(_MaximumBytesToSend)
{
    ClearContainer();
}

//------------------------------------------------
//
void FRH_EventContainer::ClearContainer(int32 StartingBufferSize)
{
    EventsInContainer = 0;
    SerializedJson.Empty(StartingBufferSize);
}

//------------------------------------------------
//
void FRH_EventContainer::OpenContainer()
{
    SerializedJson = TEXT("{ \"eventList\": [ ");
}

//------------------------------------------------
//
// all the real work happens here
FRH_EventContainer::EAddResult FRH_EventContainer::AddEventObject(TSharedRef<FRH_EventBase> HiRezEvent)
{
    const FString& TheJsonEvent = HiRezEvent->SerializeToJson();
    const int32 EventCharacterCount = TheJsonEvent.Len();
    UE_LOG(LogRallyHereEvent, Log, TEXT("The message and length. [characters = %d] [json = %s]."), EventCharacterCount, *TheJsonEvent);
    if (EventCharacterCount > MaximumBytesToSend)
    {
        // this message can't be sent
        UE_LOG(LogRallyHereEvent, Warning, TEXT("A message exceeds acceptable length - dropping it. [characers = %d] [json = %s]."), EventCharacterCount, *TheJsonEvent);
        return AddResult_EventTooLarge;
    }
    else if (SerializedJson.Len() + EventCharacterCount > MaximumBytesToSend)
    {
        UE_LOG(LogRallyHereEvent, Warning, TEXT("The buffer is full. [characters = %d]."), (SerializedJson.Len()));
        // stop here; do others later
        return AddResult_BufferFull;
    }
    // else
    EventsInContainer++;
    if (EventsInContainer > 1)
    {
        SerializedJson += TEXT(", "); // need a comma here
    }
    SerializedJson += TheJsonEvent;
    return AddResult_Success;
}

//------------------------------------------------
//
void FRH_EventContainer::CloseContainer()
{
    SerializedJson += TEXT(" ] }");
}

//------------------------------------------------
//
void FRH_EventContext::OnPostComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Success)
{
    IsCompleted = true;

    ElapsedTime = HttpRequest->GetElapsedTime();
    HTTPStatus = HttpRequest->GetStatus();
    UE_LOG(LogRallyHereEvent, Log, TEXT("OnPostComplete - HttpRequest->ProcessRequest returned [%s] in [%f] seconds."), EHttpRequestStatus::ToString(HTTPStatus), ElapsedTime);

    if (HttpResponse == nullptr)
    {
        UE_LOG(LogRallyHereEvent, Warning, TEXT("OnPostComplete - HTTPResponse==nullptr; quitting"));
        return;
    }

    ResponseCode = HttpResponse->GetResponseCode();
    ResponseString = HttpResponse->GetContentAsString();
    UE_LOG(LogRallyHereEvent, Log, TEXT("OnPostComplete - HTTPResponse returned code [%d] Message [%s]."), ResponseCode, *ResponseString);
}


//===================================================
// Event client top level implementation
//===================================================

/** Used to lock access to add/remove/find requests */
FCriticalSection FRH_EventClient::RequestLock;

// more static variables
const FString FRH_EventClient::InstallationSection = TEXT("Data");
const FString FRH_EventClient::InstallationTickResXYKey = TEXT("Data1");
const FString FRH_EventClient::InstallationJulianDateKey = TEXT("Data2");
const FString FRH_EventClient::InstallationUUIDKey = TEXT("Data3");
const FString FRH_EventClient::InstallationChecksumKey = TEXT("Data4");
const FString FRH_EventClient::InstallationPlayerIdKey = TEXT("Data5");
const FString FRH_EventClient::InstallationPlayerIdChecksumKey = TEXT("Data6");

const FString FRH_EventClient::UnknownString(TEXT("UNKNOWN"));


void FRH_EventClient::SetGameId(int32 _TSGameId)
{
    TSGameId = _TSGameId;
}

//------------------------------------------------
//
void FRH_EventClient::ConfigureEventClient(
    const FString& _EventReceiverURL
    , const int32 _GameId
    , const int32 _ConnectionTimeoutSeconds
    , const int32 _RetryCount
    , const int32 _RetryIntervalSeconds
    , const bool _SendBulkEvents
    , const int32 _BulkSendThresholdCount
    , const int32 _TimedSendThresholdSeconds
    , const int32 _GameRunningKeepaliveSeconds
)
{
    {
        TSEventReceiverURL = _EventReceiverURL;
        TSConnectionTimeoutSeconds = _ConnectionTimeoutSeconds;
        TSTryCount = _RetryCount;
        TSRetryIntervalSeconds = _RetryIntervalSeconds;
        TSSendingBulkEvents = _SendBulkEvents;
        TSBulkSendThresholdCount = _BulkSendThresholdCount;
        TSTimedSendThresholdSeconds = _TimedSendThresholdSeconds;
        TSGameRunningKeepaliveSeconds = _GameRunningKeepaliveSeconds;

        if (TSOperationState == OperationState_Unconfigured)
        {
            TSOperationState = OperationState_Running;
        }
    }
    UE_LOG(LogRallyHereEvent, Log, TEXT("ConfigureEventClient called."));
}


//------------------------------------------------
//
void FRH_EventClient::QueueEventToSend(TSharedRef<FRH_EventBase> EventToSend, bool SendImmediate)
{
    LocalScopeLock ScopeLock;
    if (SendImmediate)
    {
        PendingEvents.AddHead(EventToSend);
        TSSendImmediate = true;
    }
    else
    {
        PendingEvents.AddTail(EventToSend);
    }
}

//------------------------------------------------
//
void FRH_EventClient::ClearSentEvents(int32 CountToClear)
{
    LocalScopeLock ScopeLock;
    for (int32 idx = CountToClear; idx > 0 && PendingEvents.Num() > 0; idx--)
    {
        PendingEvents.RemoveNode(PendingEvents.GetHead());
    }
}

//------------------------------------------------
//
int32 FRH_EventClient::GetPendingEventCount()
{
    LocalScopeLock ScopeLock;
    return PendingEvents.Num();
}

void FRH_EventClient::CreateSessionUUID()
{
    TSSessionUUID = FRH_EventStandardGUID();
}


void FRH_EventClient::SetPlayerLoggedIn(bool _TSPlayerLoggedIn)
{
    TSPlayerLoggedIn = _TSPlayerLoggedIn;
    if (_TSPlayerLoggedIn)
    {
        TSOperationState = OperationState_Running;
    }
    else
    {
        TSOperationState = OperationState_Paused;
    }
}

//------------------------------------------------
//
FString FRH_EventClient::ReadApplicationVersion()
{
    FString path = FPaths::ProjectDir() + TEXT("Assembly/GameVersion.txt");

    FString VersionString = UnknownString;
    FFileHelper::LoadFileToString(VersionString, *path);
    UE_LOG(LogRallyHereEvent, Log, TEXT("Found version \"%s\"."), *VersionString);
    VersionString += IsRunningDedicatedServer() ? TEXT(" SERVER") : TEXT(" CLIENT");

    return VersionString;
}


//------------------------------------------------
//

// we methods for doing the actual work
/** https://docs.deltadna.com/advanced-integration/rest-api/
    Please keep the POST length below 5MB, anything above may be rejected.
    Bulk events should be POSTED to a different URL than single events,

    If Security Hashing is Disabled :   <COLLECT API URL>/collect/api/<ENVIRONMENT KEY>/bulk
    If Security Hashing is Enabled  :   <COLLECT API URL>/collect/api/<ENVIRONMENT KEY>/bulk/hash/<hash value>
    Please note, the events inside a bulk event list should be ordered in the order they occurred.
*/

//------------------------------------------------
//
int32 FRH_EventClient::SendAvailableEvents(bool EmptyAllEventsForShutdown)
{
    const int32 PendingRequestCount = PendingEvents.Num();
    UE_LOG(LogRallyHereEvent, Log, TEXT("SendAvailableEvents called with [%d] events pending."), GetPendingEventCount());
    TSEventSendingState = EventSendingState_ReadyToSend;

    if (PendingRequestCount == 0)
    {	// nothing to see here, move along
        UE_LOG(LogRallyHereEvent, Log, TEXT("SendAvailableEvents nothing to send."));
        return 0;
    }

    // Defaults are set to Unity; could be configured if we add overrides in constructor
    FRH_EventContainer HiRezBulkEventContainer;

    FRH_EventContext Request;

    int32 EventsToCleanUp = 0;
    if (TSSendTimestampProbe)
    {
        TSSendTimestampProbe = false;
        // send an empty bulk message
        HiRezBulkEventContainer.OpenContainer();
        HiRezBulkEventContainer.CloseContainer();

        const FString& TheJson = HiRezBulkEventContainer.GetSerializedMessage();
        const int32 TheLength = TheJson.Len();
#if UE_BUILD_SHIPPING
        UE_LOG(LogRallyHereEvent, Log, TEXT("Sending timestamp probe message"));
#else
        UE_LOG(LogRallyHereEvent, Log, TEXT("Sending timestamp probe message: [characters = %d] [json = %s]."), TheLength, *TheJson);
#endif
        Request.InitializeReqest(TSEventReceiverURL, TheJson, TSConnectionTimeoutSeconds);
    }
    else if (false == TSSendingBulkEvents) // one event at a time
    {
        UE_LOG(LogRallyHereEvent, Log, TEXT("SendAvailableEvents configuring single event."));
        EventsToCleanUp = 1;

        LocalScopeLock ScopeLock;

        // configure the request
        auto GeneralEvent = PendingEvents.GetHead()->GetValue();
        const FString& TheJson = GeneralEvent->SerializeToJson();
        const int32 TheLength = TheJson.Len();
        if (TheLength <= HiRezBulkEventContainer.GetMaximumBytesToSend())
        {
            UE_LOG(LogRallyHereEvent, Log, TEXT("Sending single message: [characters = %d] [json = %s]."), TheLength, *TheJson);
            Request.InitializeReqest(TSEventReceiverURL, TheJson, TSConnectionTimeoutSeconds);
        }
        else
        {
            UE_LOG(LogRallyHereEvent, Warning, TEXT("Dropping single message too large to send: [characters = %d] [json = %s]."), TheLength, *TheJson);
        }
    }
    else // TSSendingBulkEvents == true
    {
        UE_LOG(LogRallyHereEvent, Log, TEXT("SendAvailableEvents configuring bulk event."));

        {	// scope for lock
            HiRezBulkEventContainer.OpenContainer();
            FRH_EventContainer::EAddResult AddResult = FRH_EventContainer::AddResult_Success;

            LocalScopeLock ScopeLock; // access PendingEvents in a threadsafe manner

            auto TheEventNode = PendingEvents.GetHead();
            int32 EventsToTry = FGenericPlatformMath::Min(PendingEvents.Num(), HiRezBulkEventContainer.GetMaximumEventsToSend());

            while (EventsToTry > 0
                && AddResult == FRH_EventContainer::AddResult_Success
                && TheEventNode != nullptr
                )
            {
                EventsToTry--;
                AddResult = HiRezBulkEventContainer.AddEventObject(TheEventNode->GetValue());

                switch (AddResult)
                {
                case FRH_EventContainer::AddResult_EventTooLarge:
                {
                    // Drop this node from the list entirely
                    // This was logged inside AddEventObject
                    auto TheNextNode = TheEventNode->GetNextNode();
                    PendingEvents.RemoveNode(TheEventNode);
                    TheEventNode = TheNextNode;

                    // Keep going if we still have buffer space and events that are NOT too large to send alone.
                    AddResult = FRH_EventContainer::AddResult_Success;
                }	break;

                case FRH_EventContainer::AddResult_BufferFull:
                    // Stop here. Send any remaining events on the next cycle.
                    break;

                case FRH_EventContainer::AddResult_Success:
                {
                    // Move on to any next event available.
                    EventsToCleanUp = HiRezBulkEventContainer.GetEventsInContianer();
                    TheEventNode = TheEventNode->GetNextNode();
                }	break;
                }
            }
        }

        HiRezBulkEventContainer.CloseContainer();
        const int32 EventsInContainer = HiRezBulkEventContainer.GetEventsInContianer();
        if (EventsInContainer > 0)
        {
            UE_LOG(LogRallyHereEvent, Log, TEXT("Sending %d events in bulk."), EventsInContainer);
            Request.InitializeReqest(TSEventReceiverURL, HiRezBulkEventContainer.GetSerializedMessage(), TSConnectionTimeoutSeconds);
        }
        else
        {
            UE_LOG(LogRallyHereEvent, Log, TEXT("Processing message list resulted in empty bulk message."));
        }
    }

    if (Request.Payload.Len() <= 0)
    {
        // this is odd; something wrong with the events?
        UE_LOG(LogRallyHereEvent, Warning, TEXT("SendEventOnThread - Something odd happened. No data to send."));
        ClearSentEvents(EventsToCleanUp);
        return 0;
    }

    UE_LOG(LogRallyHereEvent, Log, TEXT("SendEventOnThread - POST'ing data length: %d."), Request.Payload.Len());

    // do this up front. If this errors out, we don't have to clean up anything.
    TSharedPtr<class IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
    if (HttpRequest == nullptr)
    {
        UE_LOG(LogRallyHereEvent, Error, TEXT("SendAvailableEvents - Failed to allocate HTTPRequest instance."));
        return 0;
    }

    //FHttpModule::Get().SetHttpTimeout(Request.TimeoutSeconds);
    HttpRequest->SetHeader(TEXT("content-type"), TEXT("application/json"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetURL(Request.HttpPostUrl);

    // We REALLY need all data to be UTF8, not TCHAR, but there is no easy way to do that right now.
    // I think we instead need a method for UTF8 translating strings carried along and passed in as value.
    HttpRequest->SetContentAsString(Request.Payload);

    /*
    // We *might* be able to do correct UTF8 characters this way.
    // This feels really hacky.
    // First test says that using SetContent(TArray<uint8>...) is a fail, probably because it is treated like a binary payload
    TArray<ANSICHAR> UTF8JsonString;
    const uint32 MaxUTF8Length = FTCHARToUTF8_Convert::ConvertedLength(*Request.Payload, Request.Payload.Len());
    UTF8JsonString.Reserve(MaxUTF8Length);
    FTCHARToUTF8_Convert::Convert(UTF8JsonString.GetData(), MaxUTF8Length, *Request.Payload, Request.Payload.Len());
    TArray<uint8> ContentPayload;
    ContentPayload.Reserve(MaxUTF8Length);
    for (auto Entry : UTF8JsonString)
        ContentPayload[Entry] = (uint8)UTF8JsonString[Entry];
    HttpRequest->SetContent(ContentPayload);
    */

    //HttpRequest->OnProcessRequestComplete().BindRaw(this, &OnPostComplete);

    TSEventSendingState = EventSendingState_SendingNow;

    // keep track of allowable attempts
    int32 CurrentTryCount = 1;

    FRH_EventSimpleTimer RetryCountDownTimer(TSRetryIntervalSeconds);

    // this is our TRY and RETRY loop
    bool NormalTimeoutSaveEvents = false;
    // how busy waits look
    const float WaitSeconds = 0.2f;
    do {
        RetryCountDownTimer.ReStart();

        // if we need to wait for a retry, but exit if the thread is asked to quit
        while (TSThreadShouldBeRunning && RetryRequested() && !RetryCountDownTimer.TimeExpired())
        {
            FPlatformProcess::Sleep(WaitSeconds);
        }

        if (!EmptyAllEventsForShutdown && !TSThreadShouldBeRunning)
        {
            break; // in case request to quit while waiting but we are not flushing, where we need to try
        }

        // we are waiting for the request to time out, complete, fail, etc.
        double LastTime = FPlatformTime::Seconds();

        // Start the wheels turning. On Windows, this includes a synchronous TCP connect, so it can fail/timeout after 30 seconds.
        HttpRequest->ProcessRequest();
        UE_LOG(LogRallyHereEvent, Log, TEXT("SendEventOnThread - POST started with status [%s]."), EHttpRequestStatus::ToString(HttpRequest->GetStatus()));

        // There seems to be a situation during shutdown where the events go out but
        // HttpRequest->GetStatus() always returns EHttpRequestStatus::Processing -- it does not TIME OUT or chagne state
        int32 BailoutTime = (EmptyAllEventsForShutdown || !TSThreadShouldBeRunning) ? 5 : Request.TimeoutSeconds + 1;
        FRH_EventSimpleTimer RedundantBailout(BailoutTime);

        // Sony and CURL starts off in ::Processing after call to ProcessRequest but XBOX begins in ::NotStarted :/
        while (
            (EHttpRequestStatus::NotStarted == HttpRequest->GetStatus() || EHttpRequestStatus::Processing == HttpRequest->GetStatus())
            && !RedundantBailout.TimeExpired()
            )
        {
            FPlatformProcess::Sleep(WaitSeconds);
            const double AppTime = FPlatformTime::Seconds();
            const float DeltaSeconds = AppTime - LastTime;
            Request.ElapsedTime += DeltaSeconds;
            LastTime = AppTime;
        }

        FHttpResponsePtr HttpResponse = HttpRequest->GetResponse();
        Request.OnPostComplete(HttpRequest, HttpResponse, HttpRequest->GetStatus() == EHttpRequestStatus::Succeeded);

        // Detect clock drift; in case we need to disable setting our own eventDateTime
        if ( HttpResponse != nullptr  /*&& TimestampMode_Measuring == TimestampMode */   )
        {
            FString ServerHttpHeaderDate = HttpResponse->GetHeader(TEXT("Date"));
            // L"Fri, 22 Jul 2022 20:07:21 GMT" -- D M Y hh:mm:ss
            FDateTime ServerHttpHeaderDateTime;
            if (FDateTime::ParseHttpDate(ServerHttpHeaderDate, ServerHttpHeaderDateTime))
            {
                FDateTime ClientUtcNow = FDateTime::UtcNow();
                ClientClockDrift = ClientUtcNow - ServerHttpHeaderDateTime; // positive value means client is ahead (includes message time)

                auto ValueInRange = [](double Min, double Max, double Value)
                {
                    return (Min < Value && Value < Max);
                };

                double MinutesOfDrift = ClientClockDrift.GetTotalMinutes();
                bool ClockDriftInRange = ValueInRange(TimestampMode_NegativeLimitMinutes, TimestampMode_PositiveLimitMinutes, MinutesOfDrift);

                if (ClockDriftInRange && !ClientTimestampMode()) // in Measure or Server
                {
                    UE_LOG(LogRallyHereEvent, Log, TEXT("SendEventOnThread - clock drift within spec: CLIENT TIMESTAMP mode: [%s]."), *ClientClockDrift.ToString());
                    TSSendingBulkEvents = TSConfigSendBulkEvents; // normal mode, use requested configuration
                    TimestampMode = TimestampMode_Client;

                    // there should only ever be one of these unless the player is mucking about with the clock during gameplay
                    SendClientClockDrift(ClientClockDrift.ToString(), FString::FromInt((int32)ClientClockDrift.GetTotalSeconds()));
                }
                else if (!ClockDriftInRange && !ServerTimestampMode()) // in client or measure
                {
                    UE_LOG(LogRallyHereEvent, Log, TEXT("SendEventOnThread - clock drift out of spec: SERVER TIMESTAMP mode: [%s]."), *ClientClockDrift.ToString());
                    TSSendingBulkEvents = false; // enforce ordering by server clock thru serializing
                    TimestampMode = TimestampMode_Server;

                    // there should only ever be one of these unless the player is mucking about with the clock during gameplay
                    SendClientClockDrift(ClientClockDrift.ToString(), FString::FromInt((int32)ClientClockDrift.GetTotalSeconds()));
                }
            }
        }

        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
        // we will consider anything 2xx to be OK
        if (Request.ResponseCode >= 200 && Request.ResponseCode < 300)
        {
            UE_LOG(LogRallyHereEvent, Log, TEXT("ProcessCompletedPost [%d] - prepping for the next one"), Request.ResponseCode);

            CurrentTryCount = 0;
            NormalTimeoutSaveEvents = false; // in case timeout is followed by success
            TSEventSendingState = EventSendingState_ReadyToSend;
        }
        else if (Request.HTTPStatus == EHttpRequestStatus::Failed_ConnectionError // TCP connect timeout
            || Request.ResponseCode == 408 // Request Timeout - the server would like to shut down this (unused) connection
            || Request.ResponseCode == 503 // Service Unavailable - the server is not ready to handle the request
            || Request.ResponseCode == 504 // Gateway Timeout - the server, while acting as a gateway or proxy, did not get a response in time from the upstream server that it needed in order to complete the request
            || Request.ResponseCode == 0   // This means there was no response at all. This can occur during shutdown, but has also occurred on first try.
            )
        {
            CurrentTryCount++;
            NormalTimeoutSaveEvents = true;
            if (CurrentTryCount <= TSTryCount)
            {
                // wait backs off by try count; could add some randomosity if needed
                RetryCountDownTimer.SetTimer(TSRetryIntervalSeconds * CurrentTryCount);
                TSEventSendingState = EventSendingState_RetryRequested;
                UE_LOG(LogRallyHereEvent, Warning, TEXT("ProcessCompletedPost - Response [%s]"), *Request.ResponseString);
                UE_LOG(LogRallyHereEvent, Warning, TEXT("ProcessCompletedPost - Request timeout: Retry # %d after %d seconds"), CurrentTryCount, (int32)RetryCountDownTimer.GetDuration());
            }
        }
        else // unrecoverable error ?
        {
            CurrentTryCount++;
            NormalTimeoutSaveEvents = false; // in case timeout is followed by success
            RetryCountDownTimer.SetTimer(TSBadStateRetryIntervalSeconds);
            TSEventSendingState = EventSendingState_RetryRequested;
            UE_LOG(LogRallyHereEvent, Error, TEXT("ProcessCompletedPost - Response [%s]"), *Request.ResponseString);
            UE_LOG(LogRallyHereEvent, Error, TEXT("ProcessCompletedPost - Unknown failure: Try # %d after %d seconds"), CurrentTryCount, (int32)RetryCountDownTimer.GetDuration());
        }


    } while (TSThreadShouldBeRunning && RetryRequested());

    // Normal timeouts are a waiting state, not a "expect continuous failure" state
    // So if thread is exiting or if success or unusual errors, then clean up.
    if (!TSThreadShouldBeRunning || !NormalTimeoutSaveEvents)
    {
        ClearSentEvents(EventsToCleanUp);
    }

    TSEventSendingState = EventSendingState_SendCycleCompleted;
    return EventsToCleanUp;
}

//=============================================
// Implementation of the FRunnable interface
//=============================================
// Do not call these functions yourself, that will happen automatically
bool FRH_EventClient::Init() // Do your setup here, allocate memory, etc.
{
    TSThreadShouldBeRunning = false;
    UE_LOG(LogRallyHereEvent, Log, TEXT("FRH_EventClient::Init called"))

    return true;
}

//=============================================
// Implementation of the FRunnable interface
//=============================================
//
// Main data processing happens here
uint32 FRH_EventClient::Run()
{
    if (false == TSEnableEvents)
    {
        return false;
    }

    TSThreadShouldBeRunning = true;

    uint32 Result = 0;

    const float InverseMaxSendRate = 0.2;

    // timers for different triggers
    FRH_EventSimpleTimer SendThresholdByTimeTimer(TSTimedSendThresholdSeconds);
    FRH_EventSimpleTimer GameRunningEventSendTimer(TSGameRunningKeepaliveSeconds);
    FRH_EventSimpleTimer AutoPauseTimer(TSAutoPauseIntervalSeconds);

    // Allow some leeway for custom client integration to configure parameters, like endpoint.
    // We do this wait after setting up the other timers so they are already running.
    FRH_EventSimpleTimer CustomClientIntegrationWaitSecondsTimer(TSCustomClientIntegrationWaitSeconds);
    while (TSThreadShouldBeRunning && !CustomClientIntegrationWaitSecondsTimer.TimeExpired() )
    {
        FPlatformProcess::Sleep(InverseMaxSendRate);
    }

    // typical infinite loop
    while (TSThreadShouldBeRunning)
    {
        if (SdkIsInitialized())
        {
            if (GameIsRunning() && !GetPlayerActive() && AutoPauseTimer.TimeExpired())
            {
                PauseGame(); // state transition
            }
            else if (GameIsPaused() && GetPlayerActive())
            {
                ResumeGame(); // state transition
            }

            if (GameIsRunning())
            {
                if (GetPlayerActive()) // application notified us; toggle and reset timer
                {
                    SetPlayerActive(false);
                    AutoPauseTimer.ReStart();
                }

                // Do we need to send a running event?
               if (GameRunningEventSendTimer.TimeExpired())
                {
                    SendGameRunning();
                    GameRunningEventSendTimer.ReStart();
                }
            }

            // check for trigger to send events singly or in bulk
            bool TimeToSendEvents = false;
            if (TSSendTimestampProbe)
            {
                TimeToSendEvents = true;
            }
            else if (TSSendingBulkEvents)
            {
                TimeToSendEvents = (GetPendingEventCount() >= TSBulkSendThresholdCount) || SendThresholdByTimeTimer.TimeExpired();
            }
            else // false == TSSendingBulkEvents
            {	// send em as we got em. Efficiency assumes events to send are not high frequency/volume.
                TimeToSendEvents = GetPendingEventCount() > 0;
            }

            if (TimeToSendEvents || TSSendImmediate)
            {
                SendAvailableEvents();
                TSSendImmediate = false;

                if (ClientTimestampMode())
                {   // whatever was configured
                    SendThresholdByTimeTimer.SetTimer(TSTimedSendThresholdSeconds);
                }
                else if (ServerTimestampMode())
                {   // in server timestamp mode, adjust
                    double SecondsPerEvent = ServerTimestampModeWait();
                    SendThresholdByTimeTimer.SetTimer(SecondsPerEvent);
                }
                // else measuring mode; wait

                // Send could have taken substantial time; timed out, retries. Let's make breathing room.
                SendThresholdByTimeTimer.ReStart();
            }
        }

        // The bulk throttle is (for now) Default_MaximumEvents/InverseMaxSendRate
        // Default_MaximumEvents is not currently externally configurable. That makes it 1000 events/sec if Default_MaximumEvents==200 and InversMaxSendRate == 0.2
        FPlatformProcess::Sleep(InverseMaxSendRate);
    }

    // Looks like a graceful shutdown; queue the normal stop
    SendGameStopped();
    SetGameEnding();
    float WhenToQuitFlushing = FPlatformTime::Seconds() + (float)Default_SecondsToTryFlushing;
    while (GetPendingEventCount() > 0 && FPlatformTime::Seconds() < (float)WhenToQuitFlushing)
    {
        SendAvailableEvents(true /*flushing*/);
    }

    return Result;
}

//=============================================
// Implementation of the FRunnable interface
//=============================================
void FRH_EventClient::Stop()
{
    // Clean up memory usage here, and make sure the Run() function stops soon
    // The main thread will be stopped until this finishes!
    UE_LOG(LogRallyHereEvent, Log, TEXT("FRH_EventClient::Stop called."))
    TSThreadShouldBeRunning = false;
}


// -----------------------------------------------------------------------------------
// Implementation of Module/Plugin interface.
// -----------------------------------------------------------------------------------

TSharedPtr<FRH_EventClient> FRH_EventClient::Instance = nullptr;

static bool IsMobileClient()
{
    if (PLATFORM_ANDROID || PLATFORM_IOS)
    {
        return true;
    }
    return false;
}

TSharedPtr<FRH_EventClient> FRH_EventClient::Get()
{
	return Instance;
}

void FRH_EventClient::InitSingleton()
{
	if(!Instance.IsValid())
	{
        // this still has to be configured!
        UE_LOG(LogRallyHereEvent, Log, TEXT("Creating event client"));
        Instance = TSharedPtr<FRH_EventClient>(new FRH_EventClient());
	}
}

void FRH_EventClient::CleanupSingleton()
{
    if (Instance.IsValid())
    {
        Instance->ShutdownThread();
        Instance = nullptr;
    }
}

void FRH_EventClient::WriteOutBrandNewDataSection(FString IniFilePath)
{

    FIntPoint ViewportSize(TSScreenWidth, TSScreenHeight);

    // store ticks because we can convert it back to a date for other messags
    FDateTime DateTime = FDateTime::UtcNow();
    const uint64 CohortTick = (uint64)DateTime.GetTicks();

    FGuid Data1Guid = FGuid(uint32((CohortTick >> 32) & 0xFFFFFFFF), uint32((CohortTick >> 0) & 0xFFFFFFFF), uint32(ViewportSize.X), uint32(ViewportSize.Y)); // ticks:resx:resy
    FGuid Data2Guid = FGuid(uint32(DateTime.GetYear()), uint32(DateTime.GetDayOfYear()), uint32(DateTime.GetHour()), uint32(DateTime.GetMinute())); // Date and UserID parts
    FGuid Data3Guid = FGuid::NewGuid(); // actual UUID
    FGuid Data4Guid = RH_DataChecksum(Data1Guid, Data2Guid, Data3Guid);

    FString Data1TicksRezXY = Data1Guid.ToString();
    FString Data2JuilianDateParts = Data2Guid.ToString();
    FString Data3InstallationUUID = Data3Guid.ToString(); // store it in compact form (like array of bytes)
    FString Data4Checksum = Data4Guid.ToString();

    GConfig->SetString(*InstallationSection, *InstallationTickResXYKey, *Data1TicksRezXY, IniFilePath);
    GConfig->SetString(*InstallationSection, *InstallationJulianDateKey, *Data2JuilianDateParts, IniFilePath);
    GConfig->SetString(*InstallationSection, *InstallationUUIDKey, *Data3InstallationUUID, IniFilePath);
    GConfig->SetString(*InstallationSection, *InstallationChecksumKey, *Data4Checksum, IniFilePath);
    GConfig->Flush(false, IniFilePath);

    // Format it correctly
    TSInstallationUUID = FRH_EventStandardGUID(Data3Guid);
}

void FRH_EventClient::CheckAndFixupPlayerId()
{
    FString LocalGameUserSettingsIni;
    FConfigCacheIni::LoadGlobalIniFile(LocalGameUserSettingsIni, TEXT("GameUserSettings"));

    FGuid Invalid;
    FString StringData5 = Invalid.ToString();
    FString StringData6 = StringData5;
    bool bFound5 = GConfig->GetString(*InstallationSection, *InstallationPlayerIdKey, StringData5, LocalGameUserSettingsIni);
    bool bFound6 = GConfig->GetString(*InstallationSection, *InstallationPlayerIdChecksumKey, StringData6, LocalGameUserSettingsIni);

    bool AllFound = bFound5 && bFound6;
    bool NoneFound = !bFound5 && !bFound6;

    // playerId tamper check
    FGuid PlayerIdChecksum = RH_StringChecksum(StringData5);
    FGuid Guid6Read; FGuid::Parse(StringData6, Guid6Read);
    bool PlayerIdIsOk = PlayerIdChecksum == Guid6Read;


    bool ReWriteData = false;
    bool SendPlayerTamperEvent = false;
    if (NoneFound)
    {
        ReWriteData = true;
        SendPlayerTamperEvent = false;
    }
    else if (AllFound && PlayerIdIsOk)
    {
        // it is possible that the player changed 'accounts' (not hirez account, but player_id)
        bool PlayerIdChanged = StringData5 != TSPlayerId;
        ReWriteData = PlayerIdChanged;
        SendPlayerTamperEvent = false;
    }
    else // if (!NoneFound || !AllFound || !PlayerIdIsOk)
    {
        SendPlayerTamperEvent = !PlayerIdIsOk;
        ReWriteData = true;
    }

    if (ReWriteData)
    {
        FString Data5Write = TSPlayerId; // we assume the player_id is updated post login
        FString Data6Write = RH_StringChecksum(Data5Write).ToString();

        GConfig->SetString(*InstallationSection, *InstallationPlayerIdKey, *Data5Write, LocalGameUserSettingsIni);
        GConfig->SetString(*InstallationSection, *InstallationPlayerIdChecksumKey, *Data6Write, LocalGameUserSettingsIni);
        GConfig->Flush(false, LocalGameUserSettingsIni);
    }

    if (SendPlayerTamperEvent)
    {
        FString Evidence = FString::Printf(
            TEXT("{ data5='%s', data6='%s' }"),
            *StringData5,
            *StringData6
        );
        SendPlayerTampered(Evidence);
    }
}

void FRH_EventClient::ChecksumCheckAndSaveScreenResolution(int32 ResolutionX, int32 ResolutionY, bool EnsureJustOnce)
{
    /* Whenever the screen is resized, this is called. This allows us to track the largest screen size.    */
    static bool HasBeenCalled = false;
    if (HasBeenCalled && EnsureJustOnce)
    {
        return;
    }
    HasBeenCalled = true; // only once per process life

    // We need to save this. We also check data integrity at the same time.
    // They must all be there or none
    FGuid Invalid;
    FString StringData1 = Invalid.ToString();
    FString StringData2 = StringData1;
    FString StringData3 = StringData2;
    FString StringData4 = StringData3;

    FString LocalGameUserSettingsIni;
    FConfigCacheIni::LoadGlobalIniFile(LocalGameUserSettingsIni, TEXT("GameUserSettings"));

    // read the strings
    bool bFound1 = GConfig->GetString(*InstallationSection, *InstallationTickResXYKey, StringData1, LocalGameUserSettingsIni);
    bool bFound2 = GConfig->GetString(*InstallationSection, *InstallationJulianDateKey, StringData2, LocalGameUserSettingsIni);
    bool bFound3 = GConfig->GetString(*InstallationSection, *InstallationUUIDKey, StringData3, LocalGameUserSettingsIni);
    bool bFound4 = GConfig->GetString(*InstallationSection, *InstallationChecksumKey, StringData4, LocalGameUserSettingsIni);

    // make guids so we can checksum check
    FGuid Guid1Read; FGuid::Parse(StringData1, Guid1Read);
    FGuid Guid2Read; FGuid::Parse(StringData2, Guid2Read);
    FGuid Guid3Read; FGuid::Parse(StringData3, Guid3Read);
    FGuid Guid4Read; FGuid::Parse(StringData4, Guid4Read);

    // check the standard data checksum
    FGuid DataChecksumCheck = RH_DataChecksum(Guid1Read, Guid2Read, Guid3Read);
    bool ChecksumIsOk = DataChecksumCheck == Guid4Read;

    // for deciding what to do
    bool AllFound = bFound1 && bFound2 && bFound3 && bFound4;
    bool NoneFound = !bFound1 && !bFound2 && !bFound3 && !bFound4;
    bool RezXYIncreased = false;
    if (AllFound)
    {
        RezXYIncreased = int32(Guid1Read.C) < ResolutionX || int32(Guid1Read.D) < ResolutionY;
    }

    if (AllFound && ChecksumIsOk && !RezXYIncreased)
    {
        // early out; nothing to see here
        return;
    }

    SetScreenWidth(ResolutionX);
    SetScreenHeight(ResolutionY);

    bool SendNewPlayerEvent = false;
    bool SendClientDeviceEvent = false;
    bool SendPlayerTamperEvent = false;

    if (!AllFound || !ChecksumIsOk)
    {
        // restart from scratch with known good data
        // this recreates TSInstallationUUID and sets TSPlayerID
        WriteOutBrandNewDataSection(LocalGameUserSettingsIni);

        SendClientDeviceEvent = true;

        // If the section is empty vs tampered, we just assume it was never there
        SendNewPlayerEvent = NoneFound;

        // Checksum was bad so individual lines were either removed or edited
        SendPlayerTamperEvent = !ChecksumIsOk && !NoneFound;
    }
    else if (RezXYIncreased) // AllFound && ChecksumIsOk
    {
        // just rewrite because the only thing that changed was the X orY
        if (int32(Guid1Read.C) < ResolutionX)
        {
            Guid1Read.C = uint32(ResolutionX);
        }
        if (int32(Guid1Read.D) < ResolutionY)
        {
            Guid1Read.D = uint32(ResolutionY);
        }

        FGuid NewChecksum = RH_DataChecksum(Guid1Read, Guid2Read, Guid3Read);

        GConfig->SetString(*InstallationSection, *InstallationTickResXYKey, *Guid1Read.ToString(), LocalGameUserSettingsIni);
        GConfig->SetString(*InstallationSection, *InstallationChecksumKey, *NewChecksum.ToString(), LocalGameUserSettingsIni);
        GConfig->Flush(false, LocalGameUserSettingsIni);
        SendClientDeviceEvent = true; // need to update
    }

    // we'll come back and improve this
    if (SendPlayerTamperEvent)
    {
        FDateTime InstallDate(uint64(Guid1Read.A) << 32 | uint64(Guid1Read.B));
        StringData1 += (DataChecksumCheck.A != Guid4Read.A) ? TEXT("+") : TEXT("-");
        StringData2 += (DataChecksumCheck.B != Guid4Read.B) ? TEXT("+") : TEXT("-");
        StringData3 += (DataChecksumCheck.C != Guid4Read.C) ? TEXT("+") : TEXT("-");
        StringData4 += (DataChecksumCheck.D != Guid4Read.D) ? TEXT("+") : TEXT("-");
        FString Evidence = FString::Printf(
            TEXT("{ installed='%s', data1='%s', data2='%s', data3='%s', data4='%s' }"),
            *InstallDate.ToIso8601(),
            *StringData1,
            *StringData2,
            *StringData3,
            *StringData4
        );
        SendPlayerTampered(Evidence);
    }

    if (SendNewPlayerEvent)
    {
        SendNewPlayer();
    }

    if (SendClientDeviceEvent)
    {
        SendClientDevice();
    }
}


FRH_EventClient::FRH_EventClient()
{
    UE_LOG(LogRallyHereEvent, Warning, TEXT("FRH_EventClient created."));

    // Do not need to criticalsection lock this because we are on the main/game thread.

    // Use GCongfig to get parameters.
    GConfig->GetInt(TEXT("EventTracking"), TEXT("try_count"), TSTryCount.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetInt(TEXT("EventTracking"), TEXT("retry_interval_seconds"), TSRetryIntervalSeconds.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetInt(TEXT("EventTracking"), TEXT("bad_state_retry_interval_seconds"), TSBadStateRetryIntervalSeconds.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetBool(TEXT("EventTracking"), TEXT("send_bulk"), TSConfigSendBulkEvents.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetInt(TEXT("EventTracking"), TEXT("bulk_send_threshold_count"), TSBulkSendThresholdCount.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetInt(TEXT("EventTracking"), TEXT("timed_send_threshold_seconds"), TSTimedSendThresholdSeconds.UnThreadSafeRef(), GRH_EventClientIni);

    TSGameRunningKeepaliveSeconds = MAX_int32; // servers
    TSAutoPauseIntervalSeconds = MAX_int32; // servers
    if (IsRunningClientOnly() || IsRunningGame()) // any sort of client
    {
        GConfig->GetInt(TEXT("EventTracking"), TEXT("game_running_keepalive_seconds"), TSGameRunningKeepaliveSeconds.UnThreadSafeRef(), GRH_EventClientIni);
        GConfig->GetInt(TEXT("EventTracking"), TEXT("auto_pause_interval_seconds"), TSAutoPauseIntervalSeconds.UnThreadSafeRef(), GRH_EventClientIni);
    }

    GConfig->GetString(TEXT("EventTracking"), TEXT("identifier_for_vendors"), TSIdentifierForVendors.UnThreadSafeRef(), GRH_EventClientIni);

    // default is PRIME CLIENT
    GConfig->GetBool(TEXT("EventTracking"), TEXT("enable"), TSEnableEvents.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetInt(TEXT("EventTracking"), TEXT("game_id"), TSGameId.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetString(TEXT("EventTracking"), TEXT("event_url"), TSEventReceiverURL.UnThreadSafeRef(), GRH_EventClientIni);
    GConfig->GetString(TEXT("EventTracking"), TEXT("project_id"), TSProjectId.UnThreadSafeRef(), GRH_EventClientIni);

    // over ride if MOBILE and these are set, but only if they are
    if (IsMobileClient())
    {
        GConfig->GetBool(TEXT("EventTracking_Mobile"), TEXT("enable"), TSEnableEvents.UnThreadSafeRef(), GRH_EventClientIni);
        GConfig->GetInt(TEXT("EventTracking_Mobile"), TEXT("game_id"), TSGameId.UnThreadSafeRef(), GRH_EventClientIni);
        GConfig->GetString(TEXT("EventTracking_Mobile"), TEXT("event_url"), TSEventReceiverURL.UnThreadSafeRef(), GRH_EventClientIni);
        GConfig->GetString(TEXT("EventTracking_Mobile"), TEXT("project_id"), TSProjectId.UnThreadSafeRef(), GRH_EventClientIni);
    }

    // [4/3/2022 rfredericksen] - This is a hacky way to stop issues when running the editor in swiss army knife mode
    if (!IsRunningGame() && !IsRunningDedicatedServer())
    {
        TSEnableEvents = false;
        UE_LOG(LogRallyHereEvent, Warning, TEXT("FRH_EventClient:: Running as commandlet so disabling event sending"));
    }
    UE_LOG(LogRallyHereEvent, Warning, TEXT("FRH_EventClient:: Event sending enabled: %s"), (TSEnableEvents==true ? TEXT("true") : TEXT("false")));

    TSClientVersion = ReadApplicationVersion();

    TSCpuType = FPlatformMisc::GetCPUBrand().TrimStartAndEnd(); // this is cleaner
    TSGpuType = FPlatformMisc::GetPrimaryGPUBrand().TrimStartAndEnd();
    TSCpuCores = FPlatformMisc::NumberOfCoresIncludingHyperthreads();

    const uint32 kMegabyte = 1024 * 1024;
    FGenericPlatformMemoryConstants Constants = FPlatformMemory::GetConstants();
    TSTotalRamMegabytes = Constants.TotalPhysical / kMegabyte;

    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    TSAvailableRamMegabytes = MemoryStats.AvailablePhysical / kMegabyte;

    CreateSessionUUID(); // sends value to FHiRezEventBase inside; threadsafe because renewed over time

    UE_LOG(LogRallyHereEvent, Log, TEXT("FRH_EventClient:: Game=%d posting to URL %s."), (int32)TSGameId, *FString(TSEventReceiverURL) );

    // could effectively be dead
    TSDeviceId = FGenericPlatformMisc::GetDeviceId();

    // See Internationalization/Culture.h
    FCultureRef Culture = FInternationalization::Get().GetCurrentLocale();
    FString ISOLanguage = Culture->GetTwoLetterISOLanguageName();
    FString ISORegion = Culture->GetRegion();
    TSUserLocale = ISOLanguage + TEXT("_") + ISORegion;

    TSUserTimeZone = FGenericPlatformMisc::GetTimeZoneId();

    TSPlatformName = FPlatformProperties::PlatformName();
    TSPlatformEnumeration = GetPlatformEnumeration();

    // For a translation of these strings to iphone models, see: https://www.theiphonewiki.com/wiki/Models
    // for a lookup of any Mac/Iphone: https://everymac.com/ultimate-mac-lookup/
    // or see "Hardware Strings" rows in this: https://en.wikipedia.org/wiki/List_of_iOS_and_iPadOS_devices
    // FPlatformMisc::GetCPUBrand() is what FPlatformMisc::GetDeviceMakeAndModel(); returns as the model (not the make)
    TSDeviceModel = FPlatformMisc::GetCPUBrand().TrimStartAndEnd();

    // if this is altered, we will update it as part of a larger action
    FString LocalGameUserSettingsIni;
    FConfigCacheIni::LoadGlobalIniFile(LocalGameUserSettingsIni, TEXT("GameUserSettings"));

    // This message is required to ensure that clientStarted is not dropped for the out of whack clients
    SendTimestampProbe();

    if (IsRunningDedicatedServer())
    {
        // server needs TSInstallationUUID, but not the other data
        if (GConfig->GetString(*InstallationSection, *InstallationUUIDKey, TSInstallationUUID.UnThreadSafeRef(), LocalGameUserSettingsIni))
        {
            // Need to format this
            TSInstallationUUID = FRH_EventStandardGUID(TSInstallationUUID.UnThreadSafeRef());
        }
        else
        {
            TSInstallationUUID = FRH_EventStandardGUID();
            GConfig->SetString(*InstallationSection, *InstallationUUIDKey, *TSInstallationUUID.UnThreadSafeRef(), LocalGameUserSettingsIni);
        }
        TSPlayerId = TSInstallationUUID;
        SendClientStarted();
    }

    /*else*/
    if (IsRunningClientOnly() || IsRunningGame()) // any sort of client
    {
        // we want to enforce immediate creation of the data section
        bool WriteFullDataSection = false == GConfig->DoesSectionExist(*InstallationSection, LocalGameUserSettingsIni);
        if (WriteFullDataSection)
        {
            WriteOutBrandNewDataSection(LocalGameUserSettingsIni);
        }

        if (GConfig->GetString(*InstallationSection, *InstallationUUIDKey, TSInstallationUUID.UnThreadSafeRef(), LocalGameUserSettingsIni))
        {
            // Need to format this
            TSInstallationUUID = FRH_EventStandardGUID(TSInstallationUUID.UnThreadSafeRef());
        }

        FString TempPlayerId;
        if (GConfig->GetString(*InstallationSection, *InstallationPlayerIdKey, TempPlayerId, LocalGameUserSettingsIni))
        {
            TSPlayerId = TempPlayerId;
        }
        else
        {
            // use TSInstallationUUID until we have the real player_id either from config or after login success
            TSPlayerId = TSInstallationUUID;
        }

        // queue the first event(s) now that we have TSInstallationUUID, TSPlayerId
        SendClientStarted();
        if (WriteFullDataSection)
        {
            SendNewPlayer();
            SendClientDevice();
        }

        //=========================================
        // instrument callbacks for clients
        //=========================================
        FCoreDelegates::PostRenderingThreadCreated.AddLambda([]()
        {
            // do this first as a fallback; will be fine for all new installs, which start in full screen
            FIntPoint ViewportSize(0, 0);
            if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
            {
                ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
            }
            FRH_EventClientInterface::ChecksumCheckAndSaveScreenResolution(ViewportSize.X, ViewportSize.Y, true); // we are not authoritative, so just once per process cycle
        });

        auto LambdaPauseGame = []() { FRH_EventClientInterface::PauseGame(); };
        FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddLambda(LambdaPauseGame);
        FCoreDelegates::ApplicationWillDeactivateDelegate.AddLambda(LambdaPauseGame);

        auto LambdaResumeGame = []() { FRH_EventClientInterface::ResumeGame(); };
        FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddLambda(LambdaResumeGame);
        FCoreDelegates::ApplicationHasReactivatedDelegate.AddLambda(LambdaResumeGame);
    }

    SetSdkInitialized(); // release the hounds

    // Constructs the actual thread object. It will begin execution immediately
    // I've set this up so that Configure() can be called after we create object.
    // That way we can create the singleton and then configure, or reconfigure, as needed.
    // The "this" pointer is the entryway to the Init(), Run(), and Stop() callbacks.
    MyRunnableThread = FRunnableThread::Create(this, TEXT("FRH_EventClient"));
    UE_LOG(LogRallyHereEvent, Log, TEXT("FRH_EventClient::MyRunnableThread created."));
}

void FRH_EventClient::ShutdownThread()
{
    if (MyRunnableThread)
    {
        UE_LOG(LogRallyHereEvent, Log, TEXT("FRH_EventClient::Shutting down MyRunnableThread."));
        // Kill() is a blocking call that waits for the thread to finish.
        // Hopefully that doesn't take too long
        MyRunnableThread->Kill();
    }
}

FRH_EventClient::~FRH_EventClient()
{
    if (MyRunnableThread)
    {
        delete MyRunnableThread;
        MyRunnableThread = nullptr;
    }
}

//=============================================
// Implementation of standard events
//=============================================

static const FString SdkName("Rally Here Event Client");
static const FString SdkVersion("0.1.0.0");

//------------------------------------------------
//
FString FRH_EventClient::GetPlatformEnumeration()
{
    /* Unity accepts these strings
        IOS_MOBILE, IOS_TABLET, IOS_TV, ANDROID, ANDROID_MOBILE, ANDROID_TABLET, ANDROID_CONSOLE,
        WINDOWS_MOBILE, WINDOWS_TABLET, BLACKBERRY_MOBILE, BLACKBERRY_TABLET, FACEBOOK, WEB,
        PC_CLIENT, MAC_CLIENT, PS3, PS4, PSVITA, XBOX360, XBOXONE, IOS, UNKNOWN, AMAZON, WIIU, SWITCH

        but docs do not show XSX or PS5

    */

    if (FCStringAnsi::Stricmp(FPlatformProperties::PlatformName(), "XboxOne") == 0)
    {
        return TEXT("XBOXONE");
    }

    if (FCStringAnsi::Stricmp(FPlatformProperties::PlatformName(), "XSX") == 0)
    {
        return TEXT("BLACKBERRY_MOBILE"); // TODO: Set back to XSX once Unity supports it.
        //return TEXT("XSX");
    }

    if (FCStringAnsi::Stricmp(FPlatformProperties::PlatformName(), "PS4") == 0)
    {
        return TEXT("PS4");
    }

    if (FCStringAnsi::Stricmp(FPlatformProperties::PlatformName(), "PS5") == 0)
    {
        return TEXT("BLACKBERRY_TABLET"); // TODO: Set back to PS5 once Unity supports it.
        //return TEXT("PS5");
    }

    if (FCStringAnsi::Stricmp(FPlatformProperties::PlatformName(), "Switch") == 0)
    {
        return TEXT("SWITCH");
    }

    if (FCStringAnsi::Stricmp(FPlatformProperties::PlatformName(), "Android") == 0)
    {
        return TEXT("ANDROID_MOBILE");
    }

    if (FCStringAnsi::Stricmp(FPlatformProperties::PlatformName(), "IOS") == 0)
    {
        return TEXT("IOS_MOBILE");
    }

    // Look for Windows anywhere, not just exact match.
    // Could be Windows, WindowsServer, WindowsClient, WindowsNoEditor
    // There is no known variant at this time with Windows somewhere not at the beginning
    if (FCStringAnsi::Stristr(FPlatformProperties::PlatformName(), "Windows") != nullptr)
    {
        return TEXT("PC_CLIENT");
    }

    return UnknownString;
}


// our database of event versions. This may be better in some other form not requiring a build, but we have to edit code to add new ones anyhow
TMap<FRH_EventClient::EEventIdentifier, FRH_EventClient::EventInformation> FRH_EventClient::EventInformationMap = {
    {EventIdentifier_gameStarted,           {"gameStarted", 1, false} }
    , {EventIdentifier_gameRunning,         {"gameRunning", 1, false} }
    , {EventIdentifier_gameEnded,           {"gameEnded", 1, false} }
    , {EventIdentifier_clientDevice,        {"clientDevice", 1, false} }
    , {EventIdentifier_newPlayer,           {"newPlayer", 1, false} }
// custom events
	, {EventIdentifier_customEvent,         {"customEvent", 1, true} }
    , {EventIdentifier_invalidEvent,        {"invalidEvent", 1, true} }
    , {EventIdentifier_clientStarted,       {"clientStarted", 1, true} }
    , {EventIdentifier_loginRequested,      {"loginRequested", 1, true} }
    , {EventIdentifier_loginFailed,         {"loginFailed", 1, true} }
    , {EventIdentifier_disconnected,        {"disconnected", 1, true} }
    , {EventIdentifier_gameResumed,         {"gameResumed", 1, true} }
    , {EventIdentifier_clientNotification,  {"clientNotification", 1, true} }
    , {EventIdentifier_levelChange,         {"levelChange", 1, true} }
    , {EventIdentifier_tutorialProgress,    {"tutorialProgress", 1, true}}
    , {EventIdentifier_objectiveProgress,   {"objectiveProgress", 1, true}}
    , {EventIdentifier_itemOrder,		    {"itemOrder", 1, true} }
    , {EventIdentifier_matchResult,         {"matchResult", 1, true} }
    , {EventIdentifier_metric,              {"metric", 1, true} }
	, {EventIdentifier_uiEvent,             {"uiEvent", 1, true} }
};


TMap<FRH_EventClient::ENotificationCategory, FString> FRH_EventClient::NotificationCategoryMap = {
    {NotificationCategory_PlayerTamper, TEXT("configuration.ini.modified")}
    , {NotificationCategory_ClockDrift, TEXT("client.clock.drift")}
};


//------------------------------------------------
//
void FRH_EventClient::SetServerStarted()
{
    SetPlayerId(TSInstallationUUID);
    SetGameRunning(); // releases the thread
}


//------------------------------------------------
//
void FRH_EventClient::SendLoggedIn(const FString& _PlayerId)
{
    SetPlayerId(_PlayerId);
    CheckAndFixupPlayerId();
    SetPlayerLoggedIn(true);
    SendGameStarted();
}

//------------------------------------------------
//
void FRH_EventClient::SendLoggedOut(const FString& _Reason)
{
    SendGameStopped();
    SetPlayerLoggedIn(false);
    // in anticipation of a new login/resume, make a new session id
    CreateSessionUUID();
}


//------------------------------------------------
//
void FRH_EventClient::NotifyThatPlayerIsNotIdle()
{
    // the Run() loop will read this and change our state; Try to keep state transitions in the main loop, if possible.
    SetPlayerActive(true);
}

//------------------------------------------------
//
void FRH_EventClient::PauseGame()
{
    if(!GameIsPaused())
    {
        SendGamePaused();
        SetGamePaused();
        // in anticipation of a new login/resume, make a new session id
        CreateSessionUUID();
    }
}

//------------------------------------------------
//
void FRH_EventClient::ResumeGame()
{
    if(!GameIsRunning())
    {
        SetPlayerActive(true);
        SendGameResumed();
        SetGameRunning();
    }
}

//------------------------------------------------
// Very special event; first out the door and jumps the queue
void FRH_EventClient::SendTimestampProbe()
{
    // the system knows what to do; will toggle this off
    TSSendTimestampProbe = true;
    TSSendImmediate = true;
}

//------------------------------------------------
//
FRH_EventBase& FRH_EventClient::GetBasicEvent(EEventIdentifier EventIdentifier, const FString& EventNameOverride)
{
    if (!EventInformationMap.Contains(EventIdentifier))
    {
        EventIdentifier = EventIdentifier_invalidEvent;
    }
    EventInformation Info = EventInformationMap[EventIdentifier];

    // these are required or optional for all events, so lets be consistent
    FRH_EventBase& BasicEvent = (*(FRH_EventBase*)(new FRH_EventBase(!EventNameOverride.IsEmpty() ? EventNameOverride : Info.EventName, ClientTimestampMode())))
        .SetStringField(TEXT("userID"), TSPlayerId) // required
        .SetStringField(TEXT("sessionID"), TSSessionUUID)
        .SetParameterStringField(TEXT("platform"), TSPlatformEnumeration)
        .SetParameterStringField(TEXT("clientVersion"), TSClientVersion) // required
        .SetParameterStringField(TEXT("sdkMethod"), "RH_" + Info.EventName); // required

    if (false == Info.IsCustom)
    {   // the standard locations based on the spec
        BasicEvent.SetIntegerField(TEXT("eventVersion"), Info.EventVersion)// required
            .SetStringField(TEXT("unityInstallationID"), TSInstallationUUID); // optional
    }
    else // lets put them in the parameter list for custom events
    {
        BasicEvent.SetParameterIntegerField("eventVersion", Info.EventVersion)
            .SetParameterStringField(TEXT("unityInstallationID"), TSInstallationUUID);
    }

    return BasicEvent;
}

//------------------------------------------------
//
void FRH_EventClient::SendClientStarted()
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_clientStarted)
        .SetParameterStringField("platformName", TSPlatformName)
        .SetParameterStringField("sdkVersion", SdkVersion)
        .SetParameterStringField("sdkName", SdkName)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendClientDevice()
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_clientDevice)
        .SetParameterStringField("idfv", TSIdentifierForVendors)
        .SetParameterStringField("cpuType", TSCpuType)
        .SetParameterIntegerField("cpuCores", TSCpuCores)
        .SetParameterStringField("gpuType", TSGpuType)
        .SetParameterIntegerField("screenResolution", TSScreenResolution)
        .SetParameterIntegerField("screenHeight", TSScreenHeight)
        .SetParameterIntegerField("screenWidth", TSScreenWidth)
        .SetParameterIntegerField("ramTotal", TSTotalRamMegabytes)
        .SetParameterIntegerField("ramAvailable", TSAvailableRamMegabytes)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendNewPlayer()
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_newPlayer)
        .SetParameterStringField("deviceModel", TSDeviceModel)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendGameStarted()
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_gameStarted)
        .SetParameterStringField("userLocale", TSUserLocale) // required
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendGameRunning()
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_gameRunning)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendGameEnded(EGameEndType GameEndType)
{
    FString GameEndParameter = UnknownString;
    switch (GameEndType)
    {
    case GameEnd_Paused: GameEndParameter = TEXT("paused"); break;
    case GameEnd_Stopped: GameEndParameter = TEXT("stopped"); break;
    case GameEnd_Crashed: GameEndParameter = TEXT("crashed"); break;
    case GameEnd_AdWatchedAndInstalled: GameEndParameter = TEXT("ad_watched_and_installed"); break;
    }

    QueueEventToSend(
        GetBasicEvent(EventIdentifier_gameEnded)
        .SetParameterStringField("sessionEndState", GameEndParameter) // required
    );

    // in anticipation of a new login/resume, make a new session id
    CreateSessionUUID();
}

//==================================================================================
// CUSTOM EVENTS
//==================================================================================

//------------------------------------------------
//
void FRH_EventClient::SendLoginRequested()
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_loginRequested)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendLoginFailed(const FString& _ErrorMessage)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_loginFailed)
        .SetParameterStringField("errorMessage", _ErrorMessage)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendDisconnected(const FString& _DisconnectReason)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_disconnected)
        .SetParameterStringField("disconnectReason", _DisconnectReason)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendGameResumed()
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_gameResumed)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendNotification(const FString& DottedCategory, const FString& Description, const FString& ErrorMessage)
{
    FRH_EventBase& BasicEvent =
        GetBasicEvent(EventIdentifier_clientNotification)
        .SetParameterStringField("category", DottedCategory);

    if (Description.Len() > 0)
    {
        BasicEvent.SetParameterStringField("description", Description);
    }

    if (ErrorMessage.Len() > 0)
    {
        BasicEvent.SetParameterStringField("errorMessage", ErrorMessage);
    }
    // assume all notifications jump the queue
    QueueEventToSendNow(BasicEvent);
}

//------------------------------------------------
//
void FRH_EventClient::SendNotification(ENotificationCategory NotificationCategory, const FString& Description, const FString& ErrorMessage)
{
    FString DottedCategory = TEXT("category.unknown");
    if (NotificationCategoryMap.Contains(NotificationCategory))
    {
        DottedCategory = NotificationCategoryMap[NotificationCategory];
    }

    SendNotification(DottedCategory, Description, ErrorMessage);
}

//------------------------------------------------
//
void FRH_EventClient::SendLevelChange(int32 TypeId, int32 SubtypeId, int32 StartLevel, int32 EndLevel, const FString& CurrencyEventId, const FString& Description)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_levelChange)
        .SetParameterIntegerField("typeID", TypeId)
        .SetParameterIntegerField("subtypeID", SubtypeId)
        .SetParameterIntegerField("startLevel", StartLevel)
        .SetParameterIntegerField("endLevel", EndLevel)
        .SetParameterStringField("currencyEventID", CurrencyEventId)
        .SetParameterStringField("description", Description)
    );
}

void FRH_EventClient::SendTutorialProgress(int32 CurrentStage, bool Dismissed)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_tutorialProgress)
        .SetParameterIntegerField("currentStage", CurrentStage)
        .SetParameterBoolField("dismissed", Dismissed)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendObjectiveProgress(
    int32 VendorId,
    int32 LootTableItemId,
    int32 ItemId,
    int32 StartProgress,
    int32 EndProgress,
    int32 PortalId,
    const FString& TimeStamp,
    const FString& OrderRefId,
    const FString& OrderId,
    const FString& OrderEntryId,
    const FString& Description
)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_objectiveProgress)
        .SetParameterIntegerField("vendorID", VendorId)
        .SetParameterIntegerField("lootTableItemID", LootTableItemId)
        .SetParameterIntegerField("itemID", ItemId)
        .SetParameterIntegerField("startProgress", StartProgress)
        .SetParameterIntegerField("endProgress", EndProgress)
        .SetParameterIntegerField("portalID", PortalId)
        .SetParameterStringField("timeStamp", TimeStamp)
        .SetParameterStringField("orderRefID", OrderRefId)
        .SetParameterStringField("orderID", OrderId)
        .SetParameterStringField("orderEntryID", OrderEntryId)
        .SetParameterStringField("description", Description)
    );

}

//------------------------------------------------
//
void FRH_EventClient::SendItemOrder(
    int32 ItemTypeId,
    int32 ItemSubtypeId,
    int32 VendorId,
    int32 LootTableItemId,
    int32 ItemId,
    int32 StartProgress,
    int32 EndProgress,
    int32 PortalId,
    const FString& TimeStamp,
    const FString& OrderRefId,
    const FString& OrderId,
    const FString& OrderEntryId,
    const FString& Description
)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_itemOrder)
        .SetParameterIntegerField("itemTypeID", ItemTypeId)
        .SetParameterIntegerField("itemSubtypeID", ItemSubtypeId)
        .SetParameterIntegerField("vendorID", VendorId)
        .SetParameterIntegerField("lootTableItemID", LootTableItemId)
        .SetParameterIntegerField("itemID", ItemId)
        .SetParameterIntegerField("startProgress", StartProgress)
        .SetParameterIntegerField("endProgress", EndProgress)
        .SetParameterIntegerField("portalID", PortalId)
        .SetParameterStringField("timeStamp", TimeStamp)
        .SetParameterStringField("orderRefID", OrderRefId)
        .SetParameterStringField("orderID", OrderId)
        .SetParameterStringField("orderEntryID", OrderEntryId)
        .SetParameterStringField("description", Description)
    );
}


//------------------------------------------------
//
void FRH_EventClient::SendServerMatchResult(const FString& PlayerInstallationUUID, const FString& PlayerSessionUUID, const FString& PlayerID, const FRH_JsonDataSet& OptionalParameters )
{
    FRH_EventBase& BasicEvent = GetBasicEvent(EventIdentifier_matchResult);
    // now we have to override the default values because this is sent from the server
    BasicEvent.SetStringField(TEXT("userID"), PlayerID)
        .SetStringField(TEXT("sessionID"), PlayerSessionUUID)
        .SetParameterStringField(TEXT("serverSessionID"), TSSessionUUID) // So we can join events sent from the instance but which use sessionID for the player session UUID.
        .SetParameterStringField(TEXT("unityInstallationID"), PlayerInstallationUUID) // we know player installationId is custom so must go into eventParams
        // and now the requested parameters
        .SetParameters(OptionalParameters);

    QueueEventToSend(BasicEvent);
}

//------------------------------------------------
//
void FRH_EventClient::SendMetric(const FString& MetricName, const FRH_JsonDataSet& OptionalParameters)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_metric)
        .SetParameterStringField(TEXT("metricName"), MetricName)
        .SetParameters(OptionalParameters)
    );
}

//------------------------------------------------
//
void FRH_EventClient::SendCustomEvent(const FString& CustomEventName, const FRH_JsonDataSet& OptionalParameters)
{
	QueueEventToSend(
		GetBasicEvent(EventIdentifier_customEvent, CustomEventName)
		.SetParameters(OptionalParameters)
	);
}


//------------------------------------------------
//
void FRH_EventClient::SendUIEvent(const FString& Category, const FString& Action, const FRH_JsonDataSet& OptionalParameters)
{
    QueueEventToSend(
        GetBasicEvent(EventIdentifier_uiEvent)
        .SetParameterStringField(TEXT("uiCategory"), Category)
        .SetParameterStringField(TEXT("uiAction"), Action)
        .SetParameters(OptionalParameters)
    );
}


//=============================================
// Implementation of static interface for use from C++, or creation of a blueprint library
//=============================================

//------------------------------------------------
// A bit more verbose than a MACRO but nicer to debug
FORCEINLINE FRH_EventClient* LoggedAndGuardedEventClientPtr(const FString Identifer)
{
    TSharedPtr<FRH_EventClient> EventClientPtr = FRH_EventClient::Get();
    if (!EventClientPtr.IsValid())
    {
        UE_LOG(LogRallyHereEvent, Log, TEXT("RallyHereEventClientInterface::%s called before initialization"), *Identifer);
        return nullptr;
    }

    return EventClientPtr.Get();
}


//------------------------------------------------
//
void FRH_EventClientInterface::GetClientData(
    FString& InstallationUUID
    , FString& SessionUUID
    , FString& ClientCurrentLanguage
    , int32& ScreenResolution
    , int32& ViewPortScreenX
    , int32& ViewPortScreenY
    , FString& BuildVersion
    , FString& DeviceModel
    , FString& CPUType
    , int32& CPUCores
    , FString& GPUType
    , int32& AvailableRamMegabytes
    , int32& TotalRamMegabytes
)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        InstallationUUID = RawEventClientPtr->GetInstallationUUID();
        SessionUUID = RawEventClientPtr->GetSessionUUID();
        ClientCurrentLanguage = RawEventClientPtr->GetUserLocale();
        ScreenResolution = RawEventClientPtr->GetScreenResolution();
        ViewPortScreenX = RawEventClientPtr->GetScreenWidth();
        ViewPortScreenY = RawEventClientPtr->GetScreenHeight();
        BuildVersion = RawEventClientPtr->GetClientVersion();
        DeviceModel = RawEventClientPtr->GetDeviceModel();
        CPUType = RawEventClientPtr->GetCpuType();
        CPUCores = RawEventClientPtr->GetCpuCores();
        GPUType = RawEventClientPtr->GetGpuType();
        AvailableRamMegabytes = RawEventClientPtr->GetAvailableRamMegabytes();
        TotalRamMegabytes = RawEventClientPtr->GetTotalRamMegabytes();
    }
}


//------------------------------------------------
//
void FRH_EventClientInterface::SendLoginRequested()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendLoginRequested();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendLoginFailed(const FString& _ErrorMessage)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendLoginFailed(_ErrorMessage);
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendLoggedIn(const FString& _PlayerId)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendLoggedIn(_PlayerId);
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendLoggedOut(const FString& _Reason)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendLoggedOut(_Reason);
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendCustomEvent(const FString& CustomEventName, const FRH_JsonDataSet& OptionalParameters)
{
	if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
	{
		RawEventClientPtr->SendCustomEvent(CustomEventName, OptionalParameters);
	}
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendDisconnected(const FString& _DisconnectReason)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendDisconnected(_DisconnectReason);
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendGamePaused()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendGamePaused();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendGameStopped()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendGameStopped();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendGameCrashed()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendGameCrashed();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendClientDevice()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendClientDevice();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendNewPlayer()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendNewPlayer();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::NotifyThatPlayerIsNotIdle()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->NotifyThatPlayerIsNotIdle();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::PauseGame()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->PauseGame();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::ResumeGame()
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->ResumeGame();
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::ChecksumCheckAndSaveScreenResolution(int32 ResolutionX, int32 ResolutionY, bool EnsureJustOnce)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->ChecksumCheckAndSaveScreenResolution(ResolutionX, ResolutionY, EnsureJustOnce);
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendLevelChange(int32 TypeId, int32 SubtypeId, int32 StartLevel, int32 EndLevel, const FString CurrencyEventId, const FString Description)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendLevelChange(TypeId, SubtypeId, StartLevel, EndLevel, CurrencyEventId, Description);
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendTutorialProgress(int32 CurrentStage, bool Dismissed)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendTutorialProgress(CurrentStage, Dismissed);
    }
}


//------------------------------------------------
//
void FRH_EventClientInterface::SendObjectiveProgress(
    int32 VendorId,
    int32 LootTableItemId,
    int32 ItemId,
    int32 StartProgress,
    int32 EndProgress,
    int32 PortalId,
    const FString& TimeStamp,
    const FString& OrderRefId,
    const FString& OrderId,
    const FString& OrderEntryId,
    const FString& Description
)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendObjectiveProgress(
            VendorId,
            LootTableItemId,
            ItemId,
            StartProgress,
            EndProgress,
            PortalId,
            TimeStamp,
            OrderRefId,
            OrderId,
            OrderEntryId,
            Description
        );
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendItemOrder(
    int32 ItemTypeId,
    int32 ItemSubtypeId,
    int32 VendorId,
    int32 LootTableItemId,
    int32 ItemId,
    int32 StartProgress,
    int32 EndProgress,
    int32 PortalId,
    const FString& TimeStamp,
    const FString& OrderRefId,
    const FString& OrderId,
    const FString& OrderEntryId,
    const FString& Description
)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendItemOrder(
            ItemTypeId,
            ItemSubtypeId,
            VendorId,
            LootTableItemId,
            ItemId,
            StartProgress,
            EndProgress,
            PortalId,
            TimeStamp,
            OrderRefId,
            OrderId,
            OrderEntryId,
            Description
        );
    }
}


//------------------------------------------------
//
void FRH_EventClientInterface::SendServerMatchResult(const FString& PlayerInstallationUUID, const FString& PlayerSessionUUID, int32 PlayerID, const FRH_JsonDataSet& OptionalParameters)
{
    SendServerMatchResult(
        FRH_EventStandardGUID(PlayerInstallationUUID),
        FRH_EventStandardGUID(PlayerSessionUUID),
        FString::Printf(TEXT("%d"), PlayerID),
        OptionalParameters
    );
}


//------------------------------------------------
//
void FRH_EventClientInterface::SendServerMatchResult(const FString& PlayerInstallationUUID, const FString& PlayerSessionUUID, const FString& PlayerID, const FRH_JsonDataSet& OptionalParameters)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendServerMatchResult(PlayerInstallationUUID, PlayerSessionUUID, PlayerID, OptionalParameters);
    }
}

//------------------------------------------------
//
void FRH_EventClientInterface::SendMetric(const FString& MetricName, const FRH_JsonDataSet& OptionalParameters)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendMetric(MetricName, OptionalParameters);
    }
}


//------------------------------------------------
//
void FRH_EventClientInterface::SendUIEvent(const FString& Category, const FString& Action, const FString& Label, double Value)
{
    if (FRH_EventClient* RawEventClientPtr = LoggedAndGuardedEventClientPtr(__func__))
    {
        RawEventClientPtr->SendUIEvent(Category, Action, Label, Value);
    }
}
