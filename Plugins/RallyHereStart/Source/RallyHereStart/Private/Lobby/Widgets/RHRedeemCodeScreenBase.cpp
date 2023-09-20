// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/Widgets/RHRedeemCodeScreenBase.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RH_GameInstanceSubsystem.h"
#include "RH_CatalogSubsystem.h"

void URHRedeemCodeScreenBase::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();
}
void URHRedeemCodeScreenBase::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();
}

void URHRedeemCodeScreenBase::RedeemCode(const FString& Code)
{
	if (IsPendingServerReply())
	{
		UE_LOG(RallyHereStart, Error, TEXT("RHRedeemCodeScreenBase::RedeemCode tried to submit while waiting"));
		return;
	}

	OnRedeemCodeSubmit();

	if (Code.IsEmpty())
	{
		UE_LOG(RallyHereStart, Warning, TEXT("RHRedeemCodeScreenBase::RedeemCode Code entered was empty"));
		OnRedeemCodeResult(false, NSLOCTEXT("RHBind", "MissingInput", "Empty Input"));
		return;
	}

	const auto GameInstanceSubsystem = Cast<URH_GameInstanceSubsystem>(MyHud->GetGameInstanceSubsystem());
	if (GameInstanceSubsystem == nullptr)
	{
		UE_LOG(RallyHereStart, Warning, TEXT("RHRedeemCodeScreenBase::RedeemCode failed to get GameInstanceSubsystem"));
		ShowGenericError();
		return;
	}

	auto* PlayerInfoSubsystem = MyHud->GetPlayerInfoSubsystem();
	if (PlayerInfoSubsystem == nullptr)
	{
		UE_LOG(RallyHereStart, Warning, TEXT("RHRedeemCodeScreenBase::RedeemCode failed to get PlayerInfoSubsystem"));
		ShowGenericError();
		return;
	}

	PlayerInfoSubsystem->GetOrCreatePlayerInfo(MyHud->GetLocalPlayerUuid())->GetPlayerInventory()->RedeemPromoCode(Code, FRH_PromoCodeResultDelegate::CreateWeakLambda(this, [this](const URH_PlayerInfo* PlayerInfo, const FString& PromoCode, const FRHAPI_PlayerOrder& PlayerOrder)
		{
			if (PlayerOrder.GetOrderId().IsEmpty())
			{
				OnRedeemCodeResult(false, NSLOCTEXT("ORDER_RESULT", "DEFAULT_PROMO_ERROR", "There was an unknown error processing this promotion code."));
			}
		}));
}

void URHRedeemCodeScreenBase::OnProcessTimeout()
{
	UE_LOG(RallyHereStart, Warning, TEXT("RHRedeemCodeScreenBase::OnProcessTimeout Timed out"));
	SubmittedCode.Empty();
	if (MyHud != nullptr && TimeoutDelay.IsValid())
	{
		MyHud->GetWorldTimerManager().ClearTimer(TimeoutDelay);
	}
	OnRedeemCodeResult(false, NSLOCTEXT("ORDER_RESULT", "PROMOTION_TIMED_OUT", "Request timed out"));
}

void URHRedeemCodeScreenBase::ShowGenericError()
{
	if (MyHud != nullptr && TimeoutDelay.IsValid())
	{
		MyHud->GetWorldTimerManager().ClearTimer(TimeoutDelay);
	}
	OnRedeemCodeResult(false, NSLOCTEXT("ORDER_RESULT", "DEFAULT_PROMO_ERROR", "There was an unknown error processing this promotion code."));
}
