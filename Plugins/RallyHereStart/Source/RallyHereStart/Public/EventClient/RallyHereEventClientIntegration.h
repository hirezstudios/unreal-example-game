#pragma once
#include "CoreMinimal.h"

#include "GameFramework/RHGameInstance.h"
#include "RH_LocalPlayerLoginSubsystem.h"
#include "RH_ConfigSubsystem.h"
#include "RH_EventClient.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRallyHereEventClientIntegration, Warning, All);

class RALLYHERESTART_API FRallyHereEventClientIntegration
{
private:
	static TSharedPtr<FRallyHereEventClientIntegration> Instance;

	static URHGameInstance* GameInstance;

public:
    FRallyHereEventClientIntegration() {}
    virtual ~FRallyHereEventClientIntegration() {}

	// functions for module start/stop
	static void InitSingleton();
	static void CleanupSingleton();

	static void SetGameInstance(URHGameInstance* InGameInstance);

    //==================================
	// Instrumenting the integration hooks
	static void OnPostRenderingThreadCreated();

	static void OnPLayerLoginStatusChanged(ULocalPlayer* Player);
	static void OnRHLoginResponse(const FRH_LoginResult& LR);

	static void OnLoginEvent_LoginRequested();
	static void GrabAppSettings(URH_ConfigSubsystem* ConfigSubsystem); // overrides to the config
};