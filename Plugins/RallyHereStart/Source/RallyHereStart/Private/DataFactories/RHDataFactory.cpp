#include "DataFactories/RHDataFactory.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "RH_LocalPlayerSubsystem.h"

URHDataFactory::URHDataFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Initialized = false;
}

void URHDataFactory::Initialize(class ARHHUDCommon* InHud)
{
	MyHud = InHud;
	Initialized = true;
}

void URHDataFactory::Uninitialize()
{
	Initialized = false;
    MyHud = nullptr;
}

void URHDataFactory::PostLogin()
{
    // override with any necessary variable resets, delegate unbinding, etc
    return;
}

void URHDataFactory::PostLogoff()
{
    // override with any necessary variable resets, delegate unbinding, etc
    return;
}

bool URHDataFactory::IsLoggedIn()
{
	if (MyHud != nullptr)
	{
		auto* LPSS = MyHud->GetLocalPlayerSubsystem();
		if (LPSS != nullptr)
		{
			return LPSS->IsLoggedIn();
		}
	}

    return false;
}
