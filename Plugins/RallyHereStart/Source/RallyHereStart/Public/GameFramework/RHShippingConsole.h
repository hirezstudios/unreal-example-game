// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/Console.h"
#include "RHShippingConsole.generated.h"

UCLASS()
class RALLYHERESTART_API URHShippingConsole : public UConsole
{
    GENERATED_UCLASS_BODY()
public:
	/**
	* Executes a console command.
	* @param Command The command to execute.
	*/
	virtual void ConsoleCommand(const FString& Command) override;

protected:

    virtual bool VerifyCommand(const FString& Command);
    virtual bool CanAccessConsole();
    virtual bool HasGMAccess() const;

	/** controls state transitions for the console */
	virtual void FakeGotoState(FName NextStateName) override;
};
