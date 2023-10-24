#include "Shared/HUD/RHHUDCommon.h"
#include "RH_LocalPlayerSubsystem.h"
#include "RH_LocalPlayerSessionSubsystem.h"
#include "Game/Widgets/RHTabScreenWidget.h"

URHTabScreenWidget::URHTabScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URHTabScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Action bindings
	ScoreboardReleased.BindDynamic(this, &URHTabScreenWidget::HandleScoreboardReleased);
	ListenForInputAction(FName("Scoreboard"), EInputEvent::IE_Released, true, ScoreboardReleased);
}

void URHTabScreenWidget::HandleScoreboardReleased()
{
	RemoveViewRoute(ScoreboardViewTag); //$$ KAB - Route names changed to Gameplay Tags
}

void URHTabScreenWidget::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	// TODO: should have some sort of manager/data factory that keeps track of the game's session instead of getting it directly from the subsystem in the widget
	// Also only do this refresh when that manager/data factory receives new data from the subsystem
	ClearScoreboard();

	URH_SessionView* GameSession = nullptr;
	if (MyHud != nullptr)
	{
		if (URH_LocalPlayerSubsystem* LPS = MyHud->GetLocalPlayerSubsystem())
		{
			if (URH_LocalPlayerSessionSubsystem* LPSS = LPS->GetSessionSubsystem())
			{
				GameSession = LPSS->GetFirstSessionByType("game");
			}
		}
	}

	if (GameSession != nullptr)
	{
		TArray<FRHAPI_SessionTeam> Teams = GameSession->GetSessionData().Teams;
		for (int i = 0; i < Teams.Num(); i++)
		{
			for (FRHAPI_SessionPlayer TeamPlayer : Teams[i].GetPlayers())
			{
				if (URH_PlayerInfo* PlayerInfo = MyHud->GetOrCreatePlayerInfo(TeamPlayer.GetPlayerUuid()))
				{
					PlayerInfo->GetLastKnownDisplayNameAsync(FTimespan(), false, ERHAPI_Platform::Anon,
						FRH_PlayerInfoGetDisplayNameDelegate::CreateWeakLambda(this, [this, TeamPlayer, i](bool bSuccess, const FString& DisplayName)
							{
								FString NameToUse = bSuccess ? DisplayName : TeamPlayer.GetPlayerUuid().ToString();
								AddPlayerToScoreboard(i, NameToUse);
							}));
				}
			}
		}
	}
}

void URHTabScreenWidget::OnShown_Implementation()
{
	Super::OnShown_Implementation();

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetShowMouseCursor(true);
	}
}

void URHTabScreenWidget::OnHide_Implementation()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetShowMouseCursor(false);
	}

	Super::OnHide_Implementation();
}