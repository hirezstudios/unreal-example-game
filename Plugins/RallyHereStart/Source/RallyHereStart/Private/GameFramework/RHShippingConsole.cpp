#include "RallyHereStart.h"
#include "GameFramework/RHShippingConsole.h"
#include "Engine/GameViewportClient.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

URHShippingConsole::URHShippingConsole(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void URHShippingConsole::ConsoleCommand(const FString& Command)
{
	FString Line;
	const TCHAR* Stream = *Command;

	bool bVerified = true;

	// iterate over the line, breaking up on |'s
	while (FParse::Line(&Stream, Line))
	{
		if (!VerifyCommand(Command))
		{
			bVerified = false;
			break;
		}
	}

	if (bVerified)
	{
		Super::ConsoleCommand(Command);
	}
}

bool URHShippingConsole::VerifyCommand(const FString& Command)
{
    return HasGMAccess();
}

bool URHShippingConsole::CanAccessConsole()
{
    return HasGMAccess();
}

bool URHShippingConsole::HasGMAccess() const
{
	// #RHTODO - PLAT-4582 - Add some sort of reasonable accomodation here for doing a privledge check?

    return false;
}

void URHShippingConsole::FakeGotoState(FName NextStateName)
{
	if (CanAccessConsole())
	{
		Super::FakeGotoState(NextStateName);
	}
	else if (ConsoleActive() && NextStateName == NAME_None)
	{
		Super::FakeGotoState(NextStateName);
	}
}
