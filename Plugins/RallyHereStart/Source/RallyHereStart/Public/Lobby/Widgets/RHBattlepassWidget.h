// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Inventory/RHBattlepass.h"
#include "Shared/Widgets/RHWidget.h"
#include "RHBattlepassWidget.generated.h"

UCLASS(Config=Game)
class RALLYHERESTART_API URHBattlepassWidget : public URHWidget
{
    GENERATED_BODY()

public:
    virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;
	virtual void OnShown_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void ShowPurchaseBattlepass();

	UFUNCTION(BlueprintCallable)
	void ShowPurchaseBattlepassTiers(int32 TierCount);

protected:
	UPROPERTY(BlueprintReadOnly)
	URHBattlepass* DisplayedBattlepass;
};