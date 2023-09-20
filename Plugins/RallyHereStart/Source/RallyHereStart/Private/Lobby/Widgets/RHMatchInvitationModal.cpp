
#include "RallyHereStart.h"
#include "Lobby/HUD/RHLobbyHUD.h"
#include "Lobby/Widgets/RHMatchInvitationModal.h"
#include "Managers/RHPopupManager.h"

void URHMatchInvitationModal::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void URHMatchInvitationModal::UninitializeWidget_Implementation()
{
	Super::UninitializeWidget_Implementation();
}

void URHMatchInvitationModal::AcceptInvite(int32 MapId)
{
	ClearCurrentInvite();
}

void URHMatchInvitationModal::AcceptInviteDefault()
{
	AcceptInvite(0);
}

void URHMatchInvitationModal::DeclineInvite()
{
	if (auto* QueueDataFactory = GetQueueDataFactory())
	{
		QueueDataFactory->DeclineMatchInvite();
	}
	CloseScreen();
}

float URHMatchInvitationModal::GetInvitationTimeRemaining() const
{
	if (MyHud != nullptr && InvitationExpireTimeout.IsValid())
	{
		return MyHud->GetWorldTimerManager().GetTimerRemaining(InvitationExpireTimeout);
	}
	return 0.f;
}

float URHMatchInvitationModal::GetInvitationTotalTimeToExpire() const
{
	if (MyHud != nullptr && InvitationExpireTimeout.IsValid())
	{
		return MyHud->GetWorldTimerManager().GetTimerRate(InvitationExpireTimeout);
	}
	return 0.f;
}

void URHMatchInvitationModal::CloseScreen()
{
	if (auto* LobbyHud = Cast<ARHLobbyHUD>(MyHud.Get()))
	{
		LobbyHud->GetPopupManager()->RemovePopup(PopupId);
	}
	ClearCurrentInvite();
}

URHQueueDataFactory* URHMatchInvitationModal::GetQueueDataFactory() const
{
	if (auto* const LobbyHud = Cast<const ARHLobbyHUD>(MyHud.Get()))
	{
		return LobbyHud->GetQueueDataFactory();
	}
	return nullptr;
}

void URHMatchInvitationModal::OnInvitationExpired()
{
	if (MyHud == nullptr || !PendingInvitation.IsValid())
	{
		return;
	}

	if (auto* LobbyHud = Cast<ARHLobbyHUD>(MyHud.Get()))
	{
		if (LobbyHud->GetPopupManager()->HasActivePopup())
		{
			// remove invitation response popup
			LobbyHud->GetPopupManager()->RemovePopup(PopupId);

			// show invitation expired popup
			const FText Error = FText::FormatNamed(
				NSLOCTEXT("RHCustomGames", "InvitationExpired", "Custom Lobby invite from {Name} has expired"),
				TEXT("Name"), LastInviterName
			);
			URHPopupManager* PopupManager = LobbyHud->GetPopupManager();
			FRHPopupConfig PopupData = FRHPopupConfig();

			PopupData.Description = Error;
			PopupData.IsImportant = true;

			FRHPopupButtonConfig& ConfirmButton = (PopupData.Buttons[PopupData.Buttons.AddDefaulted()]);
			ConfirmButton.Label = NSLOCTEXT("General", "Okay", "Okay");
			ConfirmButton.Type = ERHPopupButtonType::Confirm;
			ConfirmButton.Action.AddDynamic(PopupManager, &URHPopupManager::OnPopupCanceled);

			PopupManager->AddPopup(PopupData);
		}
	}
	ClearCurrentInvite();
}

void URHMatchInvitationModal::ClearCurrentInvite()
{
	if (!PendingInvitation.IsValid())
	{
		PendingInvitation.Invalidate();
	}
	InvitationExpireTimeout.Invalidate();
}

URH_FriendSubsystem* URHMatchInvitationModal::GetRH_LocalPlayerFriendSubsystem() const
{
	if (!MyHud.IsValid())
	{
		return nullptr;
	}

	APlayerController* PC = MyHud->GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] Failed to get owning player controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] Failed to get local player for controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	URH_LocalPlayerSubsystem* RHSS = LocalPlayer->GetSubsystem<URH_LocalPlayerSubsystem>();
	if (!RHSS)
	{
		UE_LOG(RallyHereStart, Error, TEXT("[%s] RallyHere failed to get friend subsystem for controller"), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	return RHSS->GetFriendSubsystem();
}