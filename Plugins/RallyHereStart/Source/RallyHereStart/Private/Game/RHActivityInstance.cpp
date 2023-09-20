// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RH_LocalPlayerSubsystem.h"
#include "RH_GameInstanceSubsystem.h"
#include "Game/RHActivityInstance.h"

URHActivityInstance::URHActivityInstance(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{

}

void URHActivityInstance::InitializeActivity(URHActivity* InActivity, const ARHPlayerController* InController)
{
	if (InActivity == nullptr || InController == nullptr || !InActivity->GetItemId().IsValid())
	{
		return;
	}

	Activity = InActivity;
	OwningController = InController;	
	URH_PlayerInfo* PlayerInfo = nullptr;

	URH_PlayerInfoSubsystem* PlayerInfoSubsystem = nullptr;

	auto* LocalPlayer = OwningController->GetLocalPlayer();
	if ( LocalPlayer != nullptr)
	{
		auto* LPSS = LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>();
		if (LPSS != nullptr)
		{
			PlayerInfoSubsystem = LPSS->GetPlayerInfoSubsystem();
		}
	}
	if (PlayerInfoSubsystem == nullptr)
	{
		auto* World = InController->GetWorld();
		auto* RHGI = World != nullptr && World->GetGameInstance() != nullptr ? World->GetGameInstance()->GetSubsystem<URH_GameInstanceSubsystem>() : nullptr;
		if (RHGI)
		{
			PlayerInfoSubsystem = RHGI->GetPlayerInfoSubsystem();
		}
	}

	if (PlayerInfoSubsystem)
	{
		PlayerInfo = PlayerInfoSubsystem->GetOrCreatePlayerInfo(OwningController->GetRHPlayerUuid());
	}

	if (PlayerInfo != nullptr)
	{
		Activity->GetCurrentProgress(PlayerInfo, FRH_GetInventoryCountDelegate::CreateWeakLambda(this, [this](const int32& Count)
		{
			InitialProgress = Count;
			if (Count < Activity->GetCompletionProgressCount())
			{
				InitializeTracking();
			}
		}));
	}
}
