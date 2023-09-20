// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Social/RHSocialPanelBase.h"
#include "RHSocialSearchPanel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRHSocialSearchGenericDelegate);

UCLASS()
class RALLYHERESTART_API URHSocialSearchPanel : public URHSocialPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void InitializeWidget_Implementation() override;
	virtual void UninitializeWidget_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "RH|Social Search")
	inline FText GetActiveSearchTerm() const { return ActiveSearchTerm; }

	UFUNCTION(BlueprintCallable, Category = "RH|Social Search")
	void DoSearch(FText SearchTerm);

	UFUNCTION(BlueprintImplementableEvent, Category = "RH|Social Search")
	void OnSearchComplete(const FText& SearchTerm, const FText& Error, const TArray<URHDataSocialPlayer*>& Results);

	UFUNCTION(BlueprintCallable, Category = "RH|Social Search")
	void OnOverlayClosed();
	
	UPROPERTY(BlueprintAssignable, Category = "RH|Social Search")
	FRHSocialSearchGenericDelegate OnOpen;

	UPROPERTY(BlueprintAssignable, Category = "RH|Social Search")
	FRHSocialSearchGenericDelegate OnClose;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 VisiblePlayerCount = 10;
	
private:
	URH_FriendSubsystem* GetFriendSubsystem() const;

	FText ActiveSearchTerm;
};
