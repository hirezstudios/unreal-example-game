#pragma once
#include "UObject/Object.h"
#include "RallyHereStart.h"
#include "RHDataFactory.generated.h"

class ARHHUDCommon;

UCLASS(BlueprintType)
class RALLYHERESTART_API URHDataFactory : public UObject
{
    GENERATED_UCLASS_BODY()

public:
    virtual void Initialize(class ARHHUDCommon* InHud);
    virtual void Uninitialize();

    virtual void PostLogin();
    virtual void PostLogoff();
	UFUNCTION(BlueprintPure)
    virtual bool IsLoggedIn();

	bool IsInitialized() const { return Initialized; }

protected:
    UPROPERTY()
    class ARHHUDCommon* MyHud;

private:
	bool Initialized;
};