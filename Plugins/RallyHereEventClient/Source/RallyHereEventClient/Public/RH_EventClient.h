// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "HttpModule.h"
#include "HttpManager.h"

#include "JsonGlobals.h"

#include "Policies/JsonPrintPolicy.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Policies/CondensedJsonPrintPolicy.h"

#include "Serialization/JsonTypes.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"

#include "Containers/List.h"
#include "HAL/Runnable.h"
#include "Misc/ScopeLock.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonSerializerMacros.h"
#include "RH_EventMetrics.h"


// -----------------------------------------------------------------------------------
// Implementation of Generalized Event Tracking classes. These should be moved to a plugin at some stage.
// -----------------------------------------------------------------------------------
class FRH_EventSimpleTimer
{
protected:
    double StartTime;
    double Duration;
    double EndTime;
public:
    FRH_EventSimpleTimer(double _Duration = 0);
    void SetTimer(double _Duration);
    double GetDuration() { return Duration; }
    void ReStart();
    double RemainingTime();
    bool TimeExpired();
};


// -----------------------------------------------------------------------------------
// Implementation of minimal replacement for FJsonObject.
// -----------------------------------------------------------------------------------

class FRH_JsonObject;
class FRH_JsonArray;

class FRH_JsonValue
{
public:
    enum EValueType {
        ValueType_Invalid
        , ValueType_Integer
        , ValueType_Double
        , ValueType_Bool
        , ValueType_String
        , ValueType_DateTime // will be converted into a standard format string
        , ValueType_Guid // will be converted into a standard format string
        , ValueType_Array
        , ValueType_Object
    };
    explicit FRH_JsonValue(FString _ValueName, int32 _IntegerValue) : ValueType(ValueType_Integer), ValueName(_ValueName) { IntegerValue = _IntegerValue; }
    explicit FRH_JsonValue(FString _ValueName, double _DoubleValue) : ValueType(ValueType_Double), ValueName(_ValueName) { DoubleValue = _DoubleValue; }
    explicit FRH_JsonValue(FString _ValueName, bool _BoolValue) : ValueType(ValueType_Bool), ValueName(_ValueName) { BoolValue = _BoolValue; }
    explicit FRH_JsonValue(FString _ValueName, FString _StringValue) : ValueType(ValueType_String), ValueName(_ValueName) { StringValue = _StringValue; }
    explicit FRH_JsonValue(EValueType _ValueType, FString _ValueName) : ValueType(_ValueType), ValueName(_ValueName) {}
    //- this one cannot use explicit so we can have initializer lists of derived classes
    FRH_JsonValue(const FRH_JsonValue& other) { *this = other; }

    FString SerializeNameValue() const
    {
        switch (ValueType)
        {
        case ValueType_Integer:     return FString::Printf(TEXT("\"%s\": %d"), *ValueName, IntegerValue); break;
        case ValueType_Double:      return FString::Printf(TEXT("\"%s\": %g"), *ValueName, DoubleValue); break;
        case ValueType_Bool:        return FString::Printf(TEXT("\"%s\": %s"), *ValueName, (BoolValue ? TEXT("true") : TEXT("false"))); break;
        default:
        case ValueType_Guid:
        case ValueType_DateTime:
        case ValueType_String:      return FString::Printf(TEXT("\"%s\": \"%s\""), *ValueName, *StringValue);
            break;
        }
        return TEXT("");
    }

    FString SerializeValue() const
    {
        switch (ValueType)
        {
        case ValueType_Integer:     return FString::Printf(TEXT("%d"), IntegerValue); break;
        case ValueType_Double:      return FString::Printf(TEXT("%g"), DoubleValue); break;
        case ValueType_Bool:        return FString::Printf(TEXT("%s"), (BoolValue ? TEXT("true") : TEXT("false"))); break;
        default:
        case ValueType_Guid:
        case ValueType_DateTime:
        case ValueType_String:      return FString::Printf(TEXT("\"%s\""), *StringValue);
            break;
        }
        return TEXT("");
    }

    EValueType GetType() const { return ValueType; }
    const FString& GetName() const { return ValueName; }
public:
    //-
    EValueType ValueType = ValueType_Invalid;
    FString ValueName = TEXT("");

    int32 IntegerValue = 0;
    double DoubleValue = 0;
    bool BoolValue = false;
    FString StringValue = TEXT("");
    FRH_JsonArray* ArrayValue = nullptr;
    FRH_JsonObject* ObjectValue = nullptr;
};

// We want to create a TSet<> of JsonValue instances with a key that is the name (not the whole class). Right now these are still case sensitive, just like JSON is.
struct FRH_JsonDataKeyFuncs : BaseKeyFuncs<FRH_JsonValue, FString, false /*bool bInAllowDuplicateKeys*/>
{
    static FORCEINLINE const FString& GetSetKey(const FRH_JsonValue& JsonData) { return JsonData.GetName(); }
    static FORCEINLINE bool Matches(const FString& A, const FString& B) { return A == B; }
    static FORCEINLINE uint32 GetKeyHash(const FString& Key) { return GetTypeHash(Key); }
};
typedef TSet<FRH_JsonValue, FRH_JsonDataKeyFuncs> FRH_JsonDataSet;

class FRH_JsonObject
{
public:
    FRH_JsonDataSet PropertyValues;
    FString SerializeValue()
    {
        FString SerializedObject;
        SerializedObject.Empty(4096 /*starting buffer size*/);
        SerializedObject += TEXT("{");
        int32 index = 0;
        for (const auto& Property : PropertyValues)
        {
            SerializedObject += Property.SerializeNameValue();
            if (index++)
            {
                SerializedObject += TEXT(", ");
            }
        }
        SerializedObject += TEXT("}");
        return SerializedObject;
    }
};

class FRH_JsonArray
{
public:
    TArray<FRH_JsonValue> ArrayValues;
    FString SerializeValue()
    {
        FString SerializedArray;
        SerializedArray.Empty(4096 /*starting buffer size*/);
        SerializedArray += TEXT("[");
        int32 index = 0;
        for (const auto& Value : ArrayValues)
        {
            SerializedArray += Value.SerializeValue();
            if (index++)
            {
                SerializedArray += TEXT(", ");
            }
        }
        SerializedArray += TEXT("]");
        return SerializedArray;
    }
};


// -----------------------------------------------------------------------------------
// Syntactic sugar to provide explicit type safety and simplified name specific definitions
// -----------------------------------------------------------------------------------
class FRH_JsonInteger : public FRH_JsonValue
{
public:
    FRH_JsonInteger(FString _ValueName, int32 _IntegerValue) : FRH_JsonValue(_ValueName, _IntegerValue) {}
};

class FRH_JsonDouble : public FRH_JsonValue
{
public:
    FRH_JsonDouble(FString _ValueName, double _DoubleValue) : FRH_JsonValue(_ValueName, _DoubleValue) {}
};

class FRH_JsonBool : public FRH_JsonValue
{
public:
    FRH_JsonBool(FString _ValueName, bool _BoolValue) : FRH_JsonValue(_ValueName, _BoolValue) {}
};

class FRH_JsonString : public FRH_JsonValue
{
public:
    FRH_JsonString(FString _ValueName, FString _StringValue) : FRH_JsonValue(_ValueName, _StringValue) {}
};

class FRH_JsonDateTime : public FRH_JsonValue
{
public:
    FRH_JsonDateTime(FString _ValueName, FDateTime _DateTimeValue) : FRH_JsonValue(_ValueName, _DateTimeValue.ToIso8601()) { ValueType = ValueType_DateTime;  }
};

class FRH_JsonGuid : public FRH_JsonValue
{
public:
    FRH_JsonGuid(FString _ValueName, FGuid _FGuidValue) : FRH_JsonValue(_ValueName, _FGuidValue.ToString(EGuidFormats::DigitsWithHyphens)) { ValueType = ValueType_Guid;  }
};

// -----------------------------------------------------------------------------------
// Specific attribute definitions
// -----------------------------------------------------------------------------------

// match level data
class FRH_InstanceID            : public FRH_JsonString  { public: explicit FRH_InstanceID(FString _StringValue)           : FRH_JsonString("instanceID", _StringValue) {} };
class FRH_InstanceSiteID        : public FRH_JsonInteger { public: explicit FRH_InstanceSiteID(int32 _IntegerValue)        : FRH_JsonInteger("instanceSiteID", _IntegerValue) {} };
class FRH_MatchID               : public FRH_JsonString  { public: explicit FRH_MatchID(FString _StringValue)              : FRH_JsonString("matchID", _StringValue) {} };
class FRH_MapName               : public FRH_JsonString  { public: explicit FRH_MapName(FString _StringValue)              : FRH_JsonString("mapName", _StringValue) {} };
class FRH_ModeName              : public FRH_JsonString  { public: explicit FRH_ModeName(FString _StringValue)             : FRH_JsonString("modeName", _StringValue) {} };
class FRH_MapGameID             : public FRH_JsonInteger { public: explicit FRH_MapGameID(int32 _IntegerValue)             : FRH_JsonInteger("mapGameID", _IntegerValue) {} };
class FRH_QueueID               : public FRH_JsonInteger { public: explicit FRH_QueueID(int32 _IntegerValue)               : FRH_JsonInteger("queueID", _IntegerValue) {} };
class FRH_TeamSize              : public FRH_JsonInteger { public: explicit FRH_TeamSize(int32 _IntegerValue)              : FRH_JsonInteger("teamSize", _IntegerValue) {} };
class FRH_WinningTeam           : public FRH_JsonInteger { public: explicit FRH_WinningTeam(int32 _IntegerValue)           : FRH_JsonInteger("winningTeam", _IntegerValue) {} };
class FRH_MatchStartDateTime    : public FRH_JsonDateTime{ public: explicit FRH_MatchStartDateTime(FDateTime _DateTimeValue): FRH_JsonDateTime("matchStartTime", _DateTimeValue) {} };
class FRH_MatchEndDateTime      : public FRH_JsonDateTime{ public: explicit FRH_MatchEndDateTime(FDateTime _DateTimeValue) : FRH_JsonDateTime("matchEndTime", _DateTimeValue) {} };
class FRH_MatchStartTime        : public FRH_JsonString  { public: explicit FRH_MatchStartTime(FString _StringValue)       : FRH_JsonString("matchStartTime", _StringValue) {} };
class FRH_MatchEndTime          : public FRH_JsonString  { public: explicit FRH_MatchEndTime(FString _StringValue)         : FRH_JsonString("matchEndTime", _StringValue) {} };
class FRH_MatchFubarState       : public FRH_JsonInteger { public: explicit FRH_MatchFubarState(int32 _IntegerValue)       : FRH_JsonInteger("matchFubarState", _IntegerValue) {} };
class FRH_Duration              : public FRH_JsonInteger { public: explicit FRH_Duration(int32 _IntegerValue)              : FRH_JsonInteger("duration", _IntegerValue) {} }; // seconds
class FRH_TotalRounds           : public FRH_JsonInteger { public: explicit FRH_TotalRounds(int32 _IntegerValue)           : FRH_JsonInteger("totalRounds", _IntegerValue) {} };
class FRH_TotalPlayers          : public FRH_JsonInteger { public: explicit FRH_TotalPlayers(int32 _IntegerValue)          : FRH_JsonInteger("totalPlayers", _IntegerValue) {} };
class FRH_TotalBots             : public FRH_JsonInteger { public: explicit FRH_TotalBots(int32 _IntegerValue)             : FRH_JsonInteger("totalBots", _IntegerValue) {} };
class FRH_TotalDeserters        : public FRH_JsonInteger { public: explicit FRH_TotalDeserters(int32 _IntegerValue)        : FRH_JsonInteger("totalDeserters", _IntegerValue) {} };
class FRH_TotalSurrenderPolls   : public FRH_JsonInteger { public: explicit FRH_TotalSurrenderPolls(int32 _IntegerValue)   : FRH_JsonInteger("totalSurrenderPolls", _IntegerValue) {} };
class FRH_EndedInSurrender      : public FRH_JsonBool    { public: explicit FRH_EndedInSurrender(bool _BoolValue)          : FRH_JsonBool("endedInSurrender", _BoolValue) {} };
class FRH_TotalBotBackfills     : public FRH_JsonInteger { public: explicit FRH_TotalBotBackfills(int32 _IntegerValue)     : FRH_JsonInteger("totalBotBackfills", _IntegerValue) {} };
class FRH_TotalHumanBackfills   : public FRH_JsonInteger { public: explicit FRH_TotalHumanBackfills(int32 _IntegerValue)   : FRH_JsonInteger("totalHumanBackfills", _IntegerValue) {} };

// player level data
class FRH_PlayerName            : public FRH_JsonString  { public: explicit FRH_PlayerName(FString _StringValue)           : FRH_JsonString("playerName", _StringValue) {} };
class FRH_InputType             : public FRH_JsonInteger { public: explicit FRH_InputType(int32 _IntegerValue)             : FRH_JsonInteger("inputType", _IntegerValue) {} };
class FRH_BackfillRound         : public FRH_JsonInteger { public: explicit FRH_BackfillRound(int32 _IntegerValue)         : FRH_JsonInteger("backfillRound", _IntegerValue) {} };
class FRH_BackfillTime          : public FRH_JsonString  { public: explicit FRH_BackfillTime(FString _StringValue)         : FRH_JsonString("backfillTime", _StringValue) {} };
class FRH_BackfillDateTime      : public FRH_JsonDateTime{ public: explicit FRH_BackfillDateTime(FDateTime _DateTimeValue) : FRH_JsonDateTime("backfillTime", _DateTimeValue) {} };

class FRH_AccountLevel          : public FRH_JsonInteger { public: explicit FRH_AccountLevel(int32 _IntegerValue)          : FRH_JsonInteger("accountLevel", _IntegerValue) {} };
class FRH_RankedLevel           : public FRH_JsonInteger { public: explicit FRH_RankedLevel(int32 _IntegerValue)           : FRH_JsonInteger("rankedLevel", _IntegerValue) {} };
class FRH_ClassLevel            : public FRH_JsonInteger { public: explicit FRH_ClassLevel(int32 _IntegerValue)            : FRH_JsonInteger("classLevel", _IntegerValue) {} };

class FRH_RankingID             : public FRH_JsonInteger { public: explicit FRH_RankingID(int32 _IntegerValue)             : FRH_JsonInteger("rankingID", _IntegerValue) {} };
class FRH_PrevRanking           : public FRH_JsonDouble  { public: explicit FRH_PrevRanking(double _DoubleValue)           : FRH_JsonDouble("prevRanking", _DoubleValue) {} }; // prev_ranking - TrueSkill mu
class FRH_PrevRankingVariance   : public FRH_JsonDouble  { public: explicit FRH_PrevRankingVariance(double _DoubleValue)   : FRH_JsonDouble("prevRankingVariance", _DoubleValue) {} }; // prev_ranking_variance - TrueSkill sigma

class FRH_TeamID                : public FRH_JsonInteger { public: explicit FRH_TeamID(int32 _IntegerValue)                : FRH_JsonInteger("teamID", _IntegerValue) {} };
class FRH_TaskForceID           : public FRH_JsonInteger { public: explicit FRH_TaskForceID(int32 _IntegerValue)           : FRH_JsonInteger("taskForceID", _IntegerValue) {} }; // team
class FRH_GroupID               : public FRH_JsonGuid    { public: explicit FRH_GroupID(const FGuid& _FGuidValue)          : FRH_JsonGuid("groupID", _FGuidValue) {} }; // party UUID
class FRH_ClassID               : public FRH_JsonInteger { public: explicit FRH_ClassID(int32 _IntegerValue)               : FRH_JsonInteger("classID", _IntegerValue) {} };// the rogue selected, but innacurate because can swap; either first or last
class FRH_PartyID               : public FRH_JsonInteger { public: explicit FRH_PartyID(int32 _IntegerValue)               : FRH_JsonInteger("partyID", _IntegerValue) {} }; // party id derived from the group_id
class FRH_PartySize             : public FRH_JsonInteger { public: explicit FRH_PartySize(int32 _IntegerValue)             : FRH_JsonInteger("partySize", _IntegerValue) {} };

class FRH_TimePlayed            : public FRH_JsonDouble  { public: explicit FRH_TimePlayed(double _DoubleValue)            : FRH_JsonDouble("timePlayed", _DoubleValue) {} };
class FRH_TimeAlive             : public FRH_JsonDouble  { public: explicit FRH_TimeAlive(double _DoubleValue)             : FRH_JsonDouble("timeAlive", _DoubleValue) {} };

// behavior and debugging stats
class FRH_OnTime                : public FRH_JsonInteger { public: explicit FRH_OnTime(int32 _IntegerValue)                : FRH_JsonInteger("onTime", _IntegerValue) {} };
class FRH_ConnectCount          : public FRH_JsonInteger { public: explicit FRH_ConnectCount(int32 _IntegerValue)          : FRH_JsonInteger("connectCount", _IntegerValue) {} };
class FRH_DisconnectRound       : public FRH_JsonInteger { public: explicit FRH_DisconnectRound(int32 _IntegerValue)       : FRH_JsonInteger("disconnectRound", _IntegerValue) {} };
class FRH_DeserterRounds        : public FRH_JsonInteger { public: explicit FRH_DeserterRounds(int32 _IntegerValue)        : FRH_JsonInteger("deserterRounds", _IntegerValue) {} };
class FRH_Penalties             : public FRH_JsonInteger { public: explicit FRH_Penalties(int32 _IntegerValue)             : FRH_JsonInteger("penalties", _IntegerValue) {} }; // currently 1/0
class FRH_AfkKicked             : public FRH_JsonInteger { public: explicit FRH_AfkKicked(int32 _IntegerValue)             : FRH_JsonInteger("afkKicked", _IntegerValue) {} }; // currently 1/0

// metric parameters
class FRH_Value                 : public FRH_JsonDouble { public: explicit FRH_Value(double _DoubleValue)                  : FRH_JsonDouble("metricValue", _DoubleValue) {} };
class FRH_Count                 : public FRH_JsonDouble { public: explicit FRH_Count(double _DoubleValue)                  : FRH_JsonDouble("metricCount", _DoubleValue) {} };
class FRH_Minimum               : public FRH_JsonDouble { public: explicit FRH_Minimum(double _DoubleValue)                : FRH_JsonDouble("metricMinimum", _DoubleValue) {} };
class FRH_Maximum               : public FRH_JsonDouble { public: explicit FRH_Maximum(double _DoubleValue)                : FRH_JsonDouble("metricMaximum", _DoubleValue) {} };
class FRH_Median                : public FRH_JsonDouble { public: explicit FRH_Median(double _DoubleValue)                 : FRH_JsonDouble("metricMedian", _DoubleValue) {} };
class FRH_Mean                  : public FRH_JsonDouble { public: explicit FRH_Mean(double _DoubleValue)                   : FRH_JsonDouble("metricMean", _DoubleValue) {} };
class FRH_StandardDeviation     : public FRH_JsonDouble { public: explicit FRH_StandardDeviation(double _DoubleValue)      : FRH_JsonDouble("metricStandardDeviation", _DoubleValue) {} };
class FRH_HistogramOverflow     : public FRH_JsonDouble { public: explicit FRH_HistogramOverflow(double _DoubleValue)      : FRH_JsonDouble("metricOverflow", _DoubleValue) {} };
class FRH_HistogramUnderflow    : public FRH_JsonDouble { public: explicit FRH_HistogramUnderflow(double _DoubleValue)     : FRH_JsonDouble("metricUnderflow", _DoubleValue) {} };
class FRH_Quantile25            : public FRH_JsonDouble { public: explicit FRH_Quantile25(double _DoubleValue)             : FRH_JsonDouble("metricQuantile25", _DoubleValue) {} };
class FRH_Quantile50            : public FRH_JsonDouble { public: explicit FRH_Quantile50(double _DoubleValue)             : FRH_JsonDouble("metricQuantile50", _DoubleValue) {} };
class FRH_Quantile75            : public FRH_JsonDouble { public: explicit FRH_Quantile75(double _DoubleValue)             : FRH_JsonDouble("metricQuantile75", _DoubleValue) {} };
class FRH_Quantile90            : public FRH_JsonDouble { public: explicit FRH_Quantile90(double _DoubleValue)             : FRH_JsonDouble("metricQuantile90", _DoubleValue) {} };
class FRH_Quantile95            : public FRH_JsonDouble { public: explicit FRH_Quantile95(double _DoubleValue)             : FRH_JsonDouble("metricQuantile95", _DoubleValue) {} };
class FRH_Quantile97            : public FRH_JsonDouble { public: explicit FRH_Quantile97(double _DoubleValue)             : FRH_JsonDouble("metricQuantile97", _DoubleValue) {} };
class FRH_Quantile99            : public FRH_JsonDouble { public: explicit FRH_Quantile99(double _DoubleValue)             : FRH_JsonDouble("metricQuantile99", _DoubleValue) {} };

// for ui events (e.g., click tracking)
class FRH_UIEventValue          : public FRH_JsonDouble { public: explicit FRH_UIEventValue(double _DoubleValue)           : FRH_JsonDouble("uiValue", _DoubleValue) {} };
class FRH_UIEventLabel          : public FRH_JsonString { public: explicit FRH_UIEventLabel(FString _StringValue)          : FRH_JsonString("uiLabel", _StringValue) {} };

class FRH_Category : public FString
{
public:
    FRH_Category(FString name) : FString(name.ToLower()) {}
    const FString operator<<(const FString & Other) const { return *this + TEXT(".") + Other; }
    FRH_Category operator<<(const FRH_Category & Other) const { return FRH_Category(*this + TEXT(".") + Other); }
};
// example strings without typing
static const FRH_Category kRH_CategoryClient(TEXT("client"));
static const FRH_Category kRH_CategoryScreen(TEXT("screen"));
static const FRH_Category kRH_CategoryJSON(TEXT("json"));


// -----------------------------------------------------------------------------------
// Implementation of Generalized Event base class.
// -----------------------------------------------------------------------------------
class FRH_EventBase
{
protected:

    void SetEventName(const FString& _eventName);

    void SetEventUUID();

    // to make debugging easier
    FString EventName;

    // use TSharedRef because we pass this to other things
    TSharedRef<FJsonObject> JsonEvent = TSharedRef<FJsonObject>(new FJsonObject());
    TSharedRef<FJsonObject> JsonParameters = TSharedRef<FJsonObject>(new FJsonObject());

    FString SerializedJson = TEXT("{}");

public:

    FRH_EventBase(const FString& _EventName, const bool _SetSenderEventTimestamp = true); // default to our global

    // To allow custom events, we make this public
    FRH_EventBase& SetStringField(const FString& FieldName, const FString& FieldValue);
    FRH_EventBase& SetIntegerField(const FString& FieldName, const int32 FieldValue);
    FRH_EventBase& SetNumberField(const FString& FieldName, const double FieldValue);

    // Methods for adding parameter entries
    FRH_EventBase& SetParameterStringField(const FString& FieldName, const FString& FieldValue);
    FRH_EventBase& SetParameterIntegerField(const FString& FieldName, const int32 FieldValue);
    FRH_EventBase& SetParameterBoolField(const FString& FieldName, const bool FieldValue);
    FRH_EventBase& SetParameterNumberField(const FString& FieldName, const double FieldValue);

    FRH_EventBase& SetParameters(const FRH_JsonDataSet& Parameters);

    // This is set inside the base constructor, but if we have a template object we hang on to it and just reset this for each send.
    // in the mean time, we will let the receiver set eventTimestamp because web clients can all be wrong individually
    FRH_EventBase& SetSenderEventTimestamp();

public:
    // Serialize the message to be sent. Caller can then use the member variable.
    const FString& SerializeToJson();
    const FString& GetSerializedMessage() { return SerializedJson; }
    TSharedRef<FJsonObject>& GetJsonObject() { return JsonEvent; }
};


class FRH_EventContainer
{
protected:
    // This is an object of { "eventList": [json_object_list*] }
    FString SerializedJson;
    int32 EventsInContainer;

    // https://docs.deltadna.com/advanced-integration/rest-api/
    // "Please keep the POST length below 5MB, anything above may be rejected."
    const int32 MaximumEventsToSend;
    const int32 MaximumBytesToSend;
public:
    enum EDefaults
    {
        Default_MaximumBytes = 4 * 1000 * 1000
        , Default_MaximumEvents = 200
        , Default_StartingBufferSize = 50 * 1000
    };
    int32 GetMaximumEventsToSend() { return MaximumEventsToSend; }
    int32 GetMaximumBytesToSend() { return MaximumBytesToSend; }

    FRH_EventContainer(int32 _MaximumEventsToSend = Default_MaximumEvents, int32 _MaximumBytesToSend = Default_MaximumBytes);
    ~FRH_EventContainer() {}

    void ClearContainer(int32 StartingBufferSize = Default_StartingBufferSize);

    enum EAddResult {
        AddResult_Success			// All is good
        , AddResult_BufferFull		// that message will not fit based on maximum single message size.
        , AddResult_EventTooLarge	// This event is just too big. We cant send it.
    };

    void OpenContainer();
    EAddResult AddEventObject(TSharedRef<FRH_EventBase> HiRezEvent);	// all the real work happens here
    void CloseContainer();

    // We serialized the message incrementally.
    const FString& GetSerializedMessage() { return SerializedJson; }
    int32 GetEventsInContianer() { return EventsInContainer; }
};



class FRH_EventContext
{
public:

    // Starting
    FString HttpPostUrl;
    FString Payload;
    int32 TimeoutSeconds;

    // Processing and results
    bool IsCompleted = false;
    float ElapsedTime = 0;
    EHttpRequestStatus::Type HTTPStatus = EHttpRequestStatus::NotStarted;
    int32 ResponseCode = 0;
    FString ResponseString = TEXT("");

    void InitializeReqest(const FString& _HttpPostUrl, const FString& _Payload, const int32 _TimeoutSeconds)
    {
        HttpPostUrl = _HttpPostUrl;
        Payload = _Payload;
        TimeoutSeconds = _TimeoutSeconds;
        // reset variables
        IsCompleted = false;
        ElapsedTime = 0;
        HTTPStatus = EHttpRequestStatus::NotStarted;
        ResponseCode = 0;
        ResponseString = TEXT("");
    }

    int32 GetResponseCode() { return ResponseCode; }
    FString GetResponseString() { return ResponseString; }
    EHttpRequestStatus::Type GetResponseStatus() { return HTTPStatus; }

    // callback with result to process
    void OnPostComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool Success);
};


/*=============================
https://docs.deltadna.com/advanced-integration/rest-api/

Please keep the POST length below 5MB, anything above may be rejected.
Bulk events should be POSTED to a different URL than single events,

If Security Hashing is Disabled :   <COLLECT API URL>/collect/api/<ENVIRONMENT KEY>/bulk
If Security Hashing is Enabled  :   <COLLECT API URL>/collect/api/<ENVIRONMENT KEY>/bulk/hash/<hash value>
Please note, the events inside a bulk event list should be ordered in the order they occurred.
=============================*/
class RALLYHEREEVENTCLIENT_API FRH_EventClient : public FRunnable
{
protected:

    /*=============================
    Using threadsafe variables may feel like overkill but avoids any issue with a new
    person missing out on thread safety and allows for 'random access' configuration
    (as opposed to having to fill out the entire Cofigure() signature).
    =============================*/

    // Used to lock access to add/remove/find requests
    static FCriticalSection RequestLock;

    // for multiple-statement protections
    class LocalScopeLock
    {
    private:
        FScopeLock ScopeLock;
    public:
        LocalScopeLock() : ScopeLock(&RequestLock) {}
        ~LocalScopeLock() = default;
    };

    // for individual variables
    template<class T>
    class ThreadSafeVariable
    {
    protected:
        FCriticalSection RequestLock;
        T Variable;
    public:
        ThreadSafeVariable()
        {
            Variable = T();
        }

        ThreadSafeVariable(T _Variable)
        {
            Variable = _Variable;
        }

		ThreadSafeVariable(const ThreadSafeVariable<T>& _Object)
		{
			FScopeLock ScopeLock(&RequestLock);
			Variable = _Object.Variable;
		}

        void operator=(const T& _Variable)
        {
            FScopeLock ScopeLock(&RequestLock);
            Variable = _Variable;
        }

        void operator=(const ThreadSafeVariable<T>& _Object)
        {
            FScopeLock ScopeLock(&RequestLock);
            Variable = _Object.Variable;
        }

        operator T()
        {
            FScopeLock ScopeLock(&RequestLock);
            return Variable;
        }

        T& UnThreadSafeRef()
        {
            return Variable;
        }
    };

    //=============================================
    // Implementation of the FRunnable interface
    //=============================================

    // Do not call these functions yourself, that will happen automatically
    bool Init() override; // Do your setup here, allocate memory, etc.
    uint32 Run() override; // Main data processing happens here
    void Stop() override; // Clean up any memory you allocated here

private:

    // Thread handle. Control the thread using this, with operators like Kill and Suspend
    FRunnableThread* MyRunnableThread;

    // Used to know when the thread should exit, set to false in Stop(), value read in Run()
    // NOTE: the ThreadSafeVariable initialization requires the {initializaer, list}
	// syntax so that it invokes a matching constructor rather than copy constructor
    ThreadSafeVariable<bool> TSThreadShouldBeRunning = { false };


protected:
    //=============================================
    // Implementation of the guts of event management
    //=============================================

    static const FString UnknownString;

    // We need a queue for "pending events".
    // We need a container class where we can remove from it with an example of the thing in it. TArray no likey
    TDoubleLinkedList<TSharedRef<FRH_EventBase>> PendingEvents;
    typedef TDoubleLinkedList<TSharedRef<FRH_EventBase>> TPendingEventNodePointer;

    // NOTE: the ThreadSafeVariable initialization requires the {initializaer, list}
	// syntax so that it invokes a matching constructor rather than copy constructor

    ThreadSafeVariable<int32> TSGameId = { 0 };
    ThreadSafeVariable<FString> TSEventReceiverURL = { UnknownString };
    ThreadSafeVariable<bool> TSEnableEvents = { true };
    ThreadSafeVariable<int32> TSConnectionTimeoutSeconds = { Default_ConnectionTimeout };
    ThreadSafeVariable<int32> TSTryCount = { Default_RetryCount };
    ThreadSafeVariable<int32> TSRetryIntervalSeconds = { Default_RetryIntervalSeconds };
    ThreadSafeVariable<int32> TSBadStateRetryIntervalSeconds = { Default_BadStateRetryIntervalSeconds };
    ThreadSafeVariable<int32> TSAutoPauseIntervalSeconds = { Default_AutoPauseIntervalSeconds };

    // What are our triggers for either bulk send or timed send.
    ThreadSafeVariable<bool> TSSendImmediate = { true }; // flag to force-send a priority event; on for first event
    ThreadSafeVariable<bool> TSSendTimestampProbe = { true };
    ThreadSafeVariable<bool> TSConfigSendBulkEvents = { true }; // what config wants us to be
    ThreadSafeVariable<bool> TSSendingBulkEvents = { false }; // operational status: start with single events until client clock drift is measured
    ThreadSafeVariable<int32> TSBulkSendThresholdCount = { Default_BulkSendTresholdCount };
    ThreadSafeVariable<int32> TSTimedSendThresholdSeconds = { Default_TimedSendTresholdCount };
    ThreadSafeVariable<int32> TSCustomClientIntegrationWaitSeconds = { Default_CustomClientIntegrationWaitSeconds };
    ThreadSafeVariable<int32> TSGameRunningKeepaliveSeconds = { Default_GameRunningKeepaliveSeconds };

    ThreadSafeVariable<FString> TSIdentifierForVendors = { UnknownString };
    ThreadSafeVariable<FString> TSProjectId = { UnknownString };

    ThreadSafeVariable<FString> TSDeviceModel = { UnknownString };
    ThreadSafeVariable<FString> TSCpuType = { UnknownString };
    ThreadSafeVariable<FString> TSGpuType = { UnknownString };
    ThreadSafeVariable<int32> TSCpuCores = { 1 };
    ThreadSafeVariable<int32> TSScreenResolution = { 1 };
    ThreadSafeVariable<int32> TSScreenHeight = { 1 };
    ThreadSafeVariable<int32> TSScreenWidth = { 1 };
    ThreadSafeVariable<int32> TSTotalRamMegabytes = { 1 };
    ThreadSafeVariable<int32> TSAvailableRamMegabytes = { 1 };

    ThreadSafeVariable<FString> TSPlayerId = { UnknownString };
    ThreadSafeVariable<FString> TSClientVersion = { UnknownString };
    ThreadSafeVariable<FString> TSUserLocale = { UnknownString };
    ThreadSafeVariable<FString> TSUserTimeZone = { UnknownString };
    ThreadSafeVariable<FString> TSPlatformName = { UnknownString };
    ThreadSafeVariable<FString> TSPlatformEnumeration = { UnknownString };

    ThreadSafeVariable<FString> TSDeviceId = { UnknownString };
    ThreadSafeVariable<FString> TSInstallationUUID = { UnknownString };
    ThreadSafeVariable<FString> TSSessionUUID = { UnknownString };

    ThreadSafeVariable<bool> TSPlayerActive = { true }; // toggles on and off
    bool GetPlayerActive() { return TSPlayerActive; }
    void SetPlayerActive(bool _TSPlayerActive) { TSPlayerActive = _TSPlayerActive; }

    ThreadSafeVariable<bool> TSPlayerLoggedIn = { false };
    void SetPlayerLoggedIn(bool _TSPlayerLoggedIn);
    bool GetPlayerLoggedIn() { return TSPlayerLoggedIn; }

    FString ReadApplicationVersion();
    FString GetPlatformEnumeration();

    void CreateSessionUUID();

    void ClearSentEvents(int32 CountToClear = 0);

    enum EOperationState
    {
        OperationState_Unconfigured
        , OperationState_SdkInitialized
        , OperationState_Running // logged in
        , OperationState_Paused
        , OperationState_Ending
    };
    ThreadSafeVariable<EOperationState> TSOperationState = { OperationState_Unconfigured };
    void SetSdkInitialized() { TSOperationState = OperationState_SdkInitialized; }
    bool SdkIsInitialized() { return TSOperationState >= OperationState_SdkInitialized; }

    void SetGameRunning() { TSOperationState = OperationState_Running; }
    bool GameIsRunning() { return OperationState_Running == TSOperationState; }

    void SetGamePaused() { TSOperationState = OperationState_Paused; }
    bool GameIsPaused() { return OperationState_Paused == TSOperationState; }

    void SetGameEnding() { TSOperationState = OperationState_Ending; }
    bool GameIsEnding() { return OperationState_Ending == TSOperationState; }

    // all internal
    enum EEventSendingState {
        EventSendingState_Unconfigured
        , EventSendingState_ReadyToSend
        , EventSendingState_SendingNow
        , EventSendingState_RetryRequested
        , EventSendingState_SendCycleCompleted
    };
    ThreadSafeVariable<EEventSendingState> TSEventSendingState = { EventSendingState_Unconfigured };
    bool ReadyToSend() { return EventSendingState_ReadyToSend == TSEventSendingState; }
    bool SendingNow() { return EventSendingState_SendingNow == TSEventSendingState; }
    bool RetryRequested() { return EventSendingState_RetryRequested == TSEventSendingState; }

    int32 SendAvailableEvents(bool EmptyAllEventsForShutdown = false);

    // These will be moved to protected once we have our library of specific events
    void QueueEventToSend(TSharedRef<FRH_EventBase> EventToSend, bool SendImmediate = false);
    void QueueEventToSend(FRH_EventBase* EventToSend) { QueueEventToSend(TSharedRef<FRH_EventBase>(EventToSend), false); }
    void QueueEventToSend(FRH_EventBase& EventToSend) { QueueEventToSend(TSharedRef<FRH_EventBase>(&EventToSend), false); }

    void QueueEventToSendNow(FRH_EventBase* EventToSend) { QueueEventToSend(TSharedRef<FRH_EventBase>(EventToSend), true); }
    void QueueEventToSendNow(FRH_EventBase& EventToSend) { QueueEventToSend(TSharedRef<FRH_EventBase>(&EventToSend), true); }

	static TSharedPtr<FRH_EventClient> Instance;

    void ShutdownThread();

    enum EEventIdentifier {
        // error event (custom)
        EventIdentifier_invalidEvent
        // API prescribed events
        , EventIdentifier_gameStarted
        , EventIdentifier_gameRunning
        , EventIdentifier_gameEnded
        , EventIdentifier_clientDevice
        , EventIdentifier_newPlayer
        // custom events
        , EventIdentifier_customEvent
        , EventIdentifier_clientStarted
        , EventIdentifier_loginRequested
        , EventIdentifier_loginFailed
        , EventIdentifier_disconnected
        , EventIdentifier_gameResumed
        , EventIdentifier_clientNotification
        , EventIdentifier_levelChange
        , EventIdentifier_tutorialProgress
        , EventIdentifier_objectiveProgress
        , EventIdentifier_itemOrder
        , EventIdentifier_matchResult
        , EventIdentifier_metric
        , EventIdentifier_uiEvent
    };

    struct EventInformation { FString EventName; int32 EventVersion; bool IsCustom;  };
    // our database of event versions. This may be better in some other form not requiring a build, but we have to edit code to add new ones anyhow
    static TMap<EEventIdentifier, EventInformation> EventInformationMap;

    enum ENotificationCategory {
        NotificationCategory_PlayerTamper
        , NotificationCategory_ClockDrift
    };
    static TMap<ENotificationCategory, FString> NotificationCategoryMap;

    static const FString InstallationSection;
    static const FString InstallationTickResXYKey;
    static const FString InstallationJulianDateKey;
    static const FString InstallationUUIDKey;
    static const FString InstallationChecksumKey;
    static const FString InstallationPlayerIdKey;
    static const FString InstallationPlayerIdChecksumKey;


    void WriteOutBrandNewDataSection(FString IniFilePath);
public:
    //=============================================
    // Implementation of the MODULE interface (which is public)
    //=============================================

	static void InitSingleton();
	static void CleanupSingleton();

    //=============================================
    // Implementation of the public interface for configuration and event queuing
    //=============================================
#if !UE_BUILD_SHIPPING
    bool LocalServerTest();
    void SendOneOfEverythingTest();
#endif

    FRH_EventClient();
	~FRH_EventClient();

    static TSharedPtr<FRH_EventClient> Get();

    void SetTryCount(int32 _TSTryCount) { TSTryCount = _TSTryCount; }
    int32 GetRetryCount() { return TSTryCount; }

    void SetConnectionTimeoutSeconds(int32 _TSConnectionTimeoutSeconds) { TSConnectionTimeoutSeconds = _TSConnectionTimeoutSeconds; }
    int32 GetConnectionTimeoutSeconds() { return TSConnectionTimeoutSeconds; }

    void SetRetryIntervalSeconds(int32 _TSRetryIntervalSeconds) { TSRetryIntervalSeconds = _TSRetryIntervalSeconds; }
    int32 GetRetryIntervalSeconds() { return TSRetryIntervalSeconds; }

    void SetBadStateRetryIntervalSeconds(int32 _TSBadStateRetryIntervalSeconds) { TSBadStateRetryIntervalSeconds = _TSBadStateRetryIntervalSeconds; }
    int32 GetBadStateRetryIntervalSeconds() { return TSBadStateRetryIntervalSeconds; }

    void SetSendBulkEvents(bool _TSSendBulkEvents) { TSSendingBulkEvents = _TSSendBulkEvents; }
    bool GetSendBulkEvents() { return TSSendingBulkEvents; }

    void SetBulkSendThresholdCount(int32 _TSBulkSendThresholdCount) { TSBulkSendThresholdCount = _TSBulkSendThresholdCount; }
    int32 GetBulkSendThresholdCount() { return TSBulkSendThresholdCount; }

    void SetTimedSendThresholdSeconds(int32 _TSTimedSendThresholdSeconds) { TSTimedSendThresholdSeconds = _TSTimedSendThresholdSeconds; }
    int32 GetTimedSendThresholdSeconds() { return TSTimedSendThresholdSeconds; }

    void SetGameRunningKeepaliveSeconds(int32 _TSGameRunningKeepaliveSeconds) { TSGameRunningKeepaliveSeconds = _TSGameRunningKeepaliveSeconds; }
    int32 GetGameRunningKeepaliveSeconds() { return TSGameRunningKeepaliveSeconds; }

    void SetAutoPauseIntervalSeconds(int32 _TSAutoPauseIntervalSeconds) { TSAutoPauseIntervalSeconds = _TSAutoPauseIntervalSeconds; }
    int32 GetAutoPauseIntervalSeconds() { return TSAutoPauseIntervalSeconds; }

    void SetGameId(int32 _TSGameId);
    int32 GetGameId() { return TSGameId; }

    void SetInstallationUUID(const FString _InstallationUUID) { TSInstallationUUID = _InstallationUUID; }
    FString GetInstallationUUID() { return TSInstallationUUID; }
    FString GetSessionUUID() { return TSSessionUUID; }
    FString GetUserLocale() { return TSUserLocale;  }

    FString GetDeviceModel() { return TSDeviceModel;  }
    FString GetCPUType() { return TSCpuType; }
    int32 GetCPUCores() { return TSCpuCores; }
    FString GetGPUType() { return TSGpuType; }
    FString GetClientVersion() { return TSClientVersion; }
    FString GetCpuType() { return TSCpuType; }
    int32 GetCpuCores() { return TSCpuCores; }
    FString GetGpuType() { return TSGpuType; }
    int32 GetAvailableRamMegabytes() { return TSAvailableRamMegabytes; }
    int32 GetTotalRamMegabytes() { return TSTotalRamMegabytes; }

    void GetIdentifiers(FString& InstallationUUID, FString& SessionUUID);


    void SetEventURL(const FString& _TSEventReceiverURL) { TSEventReceiverURL = _TSEventReceiverURL; }
    FString GetEventReceiverURL() { return TSEventReceiverURL; }

    void SetProjectId(const FString& _TSProjectId) { TSProjectId = _TSProjectId; }
    FString SetProjectId() { return TSProjectId; }

    void SetScreenHeight(int32 _ScreenHeight) { TSScreenHeight = _ScreenHeight; }
    int32 GetScreenHeight( ) { return TSScreenHeight; }

    void SetScreenWidth(int32 _ScreenWidth) { TSScreenWidth = _ScreenWidth; }
    int32 GetScreenWidth() { return TSScreenWidth; }

    int32 GetScreenResolution() { return TSScreenResolution;  }

    int32 GetPendingEventCount();

    void SetPlayerId(int64 _PlayerId) { SetPlayerId(FString::Printf(TEXT("%lld"), _PlayerId)); }
    void SetPlayerId(int32 _PlayerId) { SetPlayerId(FString::Printf(TEXT("%d"), _PlayerId)); }
    void SetPlayerId(const FString& _TSPlayerId) { TSPlayerId = _TSPlayerId; }
    FString GetPlayerId() { return TSPlayerId; }

    void SetUserLocale(const FString& _TSUserLocale) { TSUserLocale = _TSUserLocale; }

    void SetEnableEvents(bool _TSEnableEvents) { TSEnableEvents = _TSEnableEvents; }
    bool GetEnableEvents() { return TSEnableEvents; }

    enum ETimeStampMode
    {
        TimestampMode_Measuring
        , TimestampMode_Server
        , TimestampMode_Client
        // https://docs.unity.com/analytics/EventPayload.html
        // "Any event with a timestamp older than 31 days or newer than 24 hours will be rejected as they're outside valid boundaries."
        // Start with range that will forgive systems without auto DST adjustment.
        , TimestampMode_NegativeLimitMinutes = -65
        , TimestampMode_PositiveLimitMinutes = 65
    };
    ETimeStampMode TimestampMode = TimestampMode_Measuring; // initial state
    FTimespan ClientClockDrift;
    // Measuring and server are both server to guarantee the events are not rejected
    bool ClientTimestampMode() { return TimestampMode_Client == TimestampMode; }
    bool ServerTimestampMode() { return TimestampMode_Server == TimestampMode; }
    double ServerTimestampModeWait() {
        return FMath::Max<double>( 1, FMath::Max<double>(0, TSTimedSendThresholdSeconds) / FMath::Max<double>(1, TSBulkSendThresholdCount) );
    }

    enum EDefaults
    {
        Default_ConnectionTimeout = 30
        , Default_RetryCount = 3
        , Default_RetryIntervalSeconds = 30
        , Default_BadStateRetryIntervalSeconds = 120
        , Default_BulkSendTresholdCount = 50
        , Default_TimedSendTresholdCount = 60
        , Default_SecondsToTryFlushing = 60
        , Default_AutoPauseIntervalSeconds = 60 * 30 // 30min
#if UE_BUILD_SHIPPING
        , Default_GameRunningKeepaliveSeconds = 180
        , Default_CustomClientIntegrationWaitSeconds = 30
#else
        , Default_GameRunningKeepaliveSeconds = 30
        , Default_CustomClientIntegrationWaitSeconds = 60 // dev client startup is looong
#endif
    };

    void ConfigureEventClient(
        const FString& _EventReceiverURL
        , const int32 _GameId
        , const int32 _ConnectionTimeoutSeconds = Default_ConnectionTimeout
        , const int32 _RetryCount = Default_RetryCount
        , const int32 _RetryIntervalSeconds = Default_RetryIntervalSeconds
        , const bool _SendBulkEvents = true
        , const int32 _BulkSendThresholdCount = Default_BulkSendTresholdCount
        , const int32 _TimedSendThresholdSeconds = Default_TimedSendTresholdCount
        , const int32 _GameRunningKeepaliveSeconds = Default_GameRunningKeepaliveSeconds
    );
    void ChecksumCheckAndSaveScreenResolution(int32 ResolutionX, int32 ResolutionY, bool EnsureJustOnce = false);
    void CheckAndFixupPlayerId();

    // create an event with all the standard things we track
    FRH_EventBase& GetBasicEvent(EEventIdentifier EventIdentifier, const FString& EventNameOverride = "");

    // [5/2/2022 rfredericksen] - this was missing
    void SetServerStarted();

    // Standard Event List
    void SendClientDevice();
    void SendNewPlayer();
    void SendGameStarted();
    void SendGameRunning();
    enum EGameEndType {GameEnd_Paused, GameEnd_Stopped, GameEnd_Crashed, GameEnd_AdWatchedAndInstalled};
    void SendGameEnded(EGameEndType GameEndType = GameEnd_Stopped);
    // syntactic sugar
    void SendGamePaused() { SendGameEnded(GameEnd_Paused); };
    void SendGameStopped() { SendGameEnded(GameEnd_Stopped); };
    void SendGameCrashed() { SendGameEnded(GameEnd_Crashed); };
    void SendGameAdWatchedAndInstalled() { SendGameEnded(GameEnd_AdWatchedAndInstalled); };


    // custom events
    void SendTimestampProbe(); // send of an empty (or other noop) event so we can get timestamp to measure clock drift
    void SendClientStarted(); // standard is 'sdkStart' and we can't implement that correctly
    void SendLoginRequested();
    void SendLoginFailed(const FString& _ErrorMessage);

    void SendLoggedIn(const FString& _PlayerId);
    void SendLoggedOut(const FString& _Reason);
    void SendDisconnected(const FString& _DisconnectReason);

    void SendGameResumed();

    // manage state
    void NotifyThatPlayerIsNotIdle();
    void PauseGame();
    void ResumeGame();

    // tools
    void SendNotification(const FString& DottedCategory, const FString& Description = TEXT(""), const FString& ErrorMessage = TEXT(""));
    void SendNotification(ENotificationCategory NotificationCategory, const FString& Description = TEXT(""), const FString& ErrorMessage = TEXT(""));
    // known ones
    void SendPlayerTampered(const FString& Description) { SendNotification(NotificationCategory_PlayerTamper, Description); };
    void SendClientClockDrift(const FString& Description, const FString& OffsetSeconds) { SendNotification(NotificationCategory_ClockDrift, Description, OffsetSeconds); };


    // Level Change Type ultimately maps to XP table mapping XP to levels
    // TypeId mapd to the XPTable
    // Example: battle pass level table
    // SubtypeId is the specific thing using that table
    void SendLevelChange(int32 TypeId, int32 SubtypeId, int32 StartLevel, int32 EndLevel, const FString& CurrencyEventId, const FString& Description);

    void SendTutorialProgress(int32 CurrentStage, bool Dismissed);

    // these are daily challenges, contracts, and other goal oriented tasks; generally yielding some sort of reward
    void SendObjectiveProgress(
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
    );

    void SendItemOrder(
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
    );

    void SendServerMatchResult(const FString& PlayerInstallationUUID, const FString& PlayerSessionUUID, const FString& PlayerID, const FRH_JsonDataSet& OptionalParameters);

    // MetricName is game specific identifer string, but is recommended to be "dotted.lower.case";
    // Examples include "frame.rate.ms", "ping.time.ms"
    // Optional Parameters are defined as a group
    void SendMetric(const FString& MetricName, const FRH_JsonDataSet& OptionalParameters);

	// Custom Event with name then json payload
	void SendCustomEvent(const FString& CustomEventName, const FRH_JsonDataSet& OptionalParameters);

    // Category is game specific identifer string, but is recommended to be "dotted.lower.case";
    // Possible category examples include "screen.characterselect", "screen.jsonpanel"
    // Optional Parameters are defined as a group and include a string label (description) and a double value
    void SendUIEvent(const FString& Category, const FString& Action, const FString& Label, double Value)
    {
        SendUIEvent(Category, Action, { FRH_UIEventLabel(Label), FRH_UIEventValue(Value) });
    }
    void SendUIEvent(const FString& Category, const FString& Action, const FString& Label)
    {
        SendUIEvent(Category, Action, { FRH_UIEventLabel(Label) });
    }
    void SendUIEvent(const FString& Category, const FString& Action, double Value)
    {
        SendUIEvent(Category, Action, { FRH_UIEventValue(Value) });
    }
    void SendUIEvent(const FString& Category, const FString& Action, const FRH_JsonDataSet& OptionalParameters = {});
};

//=============================================
// We will move this to be the publicly visible interface.
// This interface is designed for use in a BluePrint Library.
// Some of these event sends may need to be shielded from the end user.
//=============================================

class RALLYHEREEVENTCLIENT_API FRH_EventClientInterface
{
protected:

public:


    static void GetClientData(
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
    );

    // HiRez has a plugin that automates these
    static void SendLoginRequested();
    static void SendLoginFailed(const FString& _ErrorMessage);
    static void SendLoggedIn(const FString& _PlayerId);
    static void SendLoggedOut(const FString& _Reason);
    static void SendDisconnected(const FString& _DisconnectReason);
    static void SendGamePaused(); // can also do this manually
    static void SendGameStopped(); // sent when the client shuts down/player logs out
    static void SendGameCrashed();
    static void SendClientDevice();
    static void SendNewPlayer();

    // Events for the game client to instrument as needed
    static void NotifyThatPlayerIsNotIdle(); // manages transition to PAUSED state via activity timer
    static void PauseGame(); // manages transition to PAUSED
    static void ResumeGame(); // manages transition to RUNNING
    static void ChecksumCheckAndSaveScreenResolution(int32 ResolutionX, int32 ResolutionY, bool EnsureJustOnce = false);
    static void SendLevelChange(int32 TypeId, int32 SubtypeId, int32 StartLevel, int32 EndLevel, const FString CurrencyEventId, const FString Description);
    static void SendTutorialProgress(int32 CurrentStage, bool Dismissed);
    static void SendObjectiveProgress(
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
    );
    static void SendItemOrder(
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
    );

    static void SendServerMatchResult(const FString& PlayerInstallationUUID, const FString& PlayerSessionUUID, int32 PlayerID, const FRH_JsonDataSet& OptionalParameters);
    static void SendServerMatchResult(const FString& PlayerInstallationUUID, const FString& PlayerSessionUUID, const FString& PlayerID, const FRH_JsonDataSet& OptionalParameters);

    static void SendUIEvent(const FString& Category, const FString& Action, const FString& Label = FString(), double Value = 0);

	static void SendCustomEvent(const FString& CustomEventName, const FRH_JsonDataSet& OptionalParameters);
    static void SendMetric(const FString& MetricName, const FRH_JsonDataSet& OptionalParameters);

    template <typename COUNTER_TYPE, typename VALUE_TYPE>
    static void SendMetric(const FString& MetricName, TRH_HistogramMetrics<COUNTER_TYPE, VALUE_TYPE>& HistogramMetrics)
    {
        FRH_JsonDataSet OptionalParameters{
            FRH_Count(HistogramMetrics.GetValueCount())
            , FRH_Minimum(HistogramMetrics.GetMinimum())
            , FRH_Maximum(HistogramMetrics.GetMaximum())
            , FRH_Mean(HistogramMetrics.GetBWPMean())
            , FRH_StandardDeviation(HistogramMetrics.GetBWPStandardDeviation())
            , FRH_HistogramUnderflow(HistogramMetrics.GetUnderflowCount())
            , FRH_HistogramOverflow(HistogramMetrics.GetOverflowCount())
        };

        // if there is no variance, don't send these
        if (HistogramMetrics.GetStandardDeviation() > 0)
        {
            OptionalParameters.Add(FRH_Median(HistogramMetrics.GetMedian()));
            OptionalParameters.Add(FRH_Quantile25(HistogramMetrics.GetQuantileValue(25)));
            OptionalParameters.Add(FRH_Quantile50(HistogramMetrics.GetQuantileValue(50)));
            OptionalParameters.Add(FRH_Quantile75(HistogramMetrics.GetQuantileValue(75)));
            OptionalParameters.Add(FRH_Quantile90(HistogramMetrics.GetQuantileValue(90)));
            OptionalParameters.Add(FRH_Quantile95(HistogramMetrics.GetQuantileValue(95)));
            OptionalParameters.Add(FRH_Quantile97(HistogramMetrics.GetQuantileValue(97)));
            OptionalParameters.Add(FRH_Quantile99(HistogramMetrics.GetQuantileValue(99)));
        }

        SendMetric(MetricName, OptionalParameters);
    }

};
