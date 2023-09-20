// Copyright 2016-2022 Hi-Rez Studios, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlayerExp_MatchReportSender.generated.h"

class FJsonObject;

UCLASS(Config=Game)
class RALLYHERESTART_API UPlayerExp_MatchReportSender : public UObject
{
    GENERATED_BODY()

public:
    UPlayerExp_MatchReportSender(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    void DoInit();
    void DoCleanup();

	virtual void RequestSendReport(class UGameInstance* GameInstance, const TSharedRef<FJsonObject>& InReportJsonObject);

protected:
    virtual void Init();
    virtual void Cleanup();

    UPROPERTY()
    bool bInitialized;
};