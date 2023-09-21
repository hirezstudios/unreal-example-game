// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RHGamepadLookSpeedManager.h"
#include "RHGamepadCurvedLookSpeedManager.generated.h"

class UCurveVector;

/**
 * 
 */
UCLASS(Blueprintable)
class RALLYHERESTART_API URHGamepadCurvedLookSpeedManager : public URHGamepadLookSpeedManager
{
	GENERATED_BODY()
	
public:

	URHGamepadCurvedLookSpeedManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void Init() override;
    FVector2D UpdateGamepadLook(FVector2D GamepadInput, float DeltaTime) override;

    UFUNCTION(exec)
    void SetOuterDeadZone(float NewZoneX, float NewZoneY);
    UFUNCTION(exec)
    void SetBoostMultiplier(float NewBoostX, float NewBoostY);
    UFUNCTION(exec)
    void SetBoostAcceleration(float NewAccel);
    UFUNCTION(exec)
    void SetBaseVelocityCurve(FName TestCurveName);
    UFUNCTION(exec)
    void PrintLookSpeedParameters();

protected:
    //Changes look speed management to have a one-to-one mapping from stick tilt to rotational velocity until the outer dead zone is reached,
    //at which point the velocity accelerates to a boosted level.
    UPROPERTY(EditDefaultsOnly, Category = "Look Speed")
    bool bUseBaseVelocity;

    UPROPERTY(EditDefaultsOnly, Category = "Curve|BaseVelocity", meta = (EditCondition = "bUseBaseVelocity"))
    UCurveVector* LookBaseVelocityCurve;

    UPROPERTY(EditDefaultsOnly, Category = "Curve|BaseVelocity", meta = (EditCondition = "bUseBaseVelocity"))
    FVector2D BoostThreshold;
    UPROPERTY(EditDefaultsOnly, Category = "Curve|BaseVelocity", meta = (EditCondition = "bUseBaseVelocity"))
    FVector2D BoostMultiplier;
    UPROPERTY(EditDefaultsOnly, Category = "Curve|BaseVelocity", meta = (EditCondition = "bUseBaseVelocity"))
    float BoostAcceleration;

	// These Max Velocity curves describe the max velocity the camera rotates at
	// at a given input (-1 to 1).
	UPROPERTY(EditDefaultsOnly, Category = "Curve|No Base", meta = (EditCondition = "!bUseBaseVelocity"))
	UCurveVector* MouseLookMaxVelocityCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Curve|No Base", meta = (EditCondition = "!bUseBaseVelocity"))
	UCurveVector* MouseLookMaxVelocityADSCurve;

	UPROPERTY(EditAnywhere, Category = Settings)
	FVector2D BaseLookRateScale;
	UPROPERTY(EditAnywhere, Category = Settings)
	float TurnRateMultiplier;
	UPROPERTY(EditAnywhere, Category = Settings)
	float LookUpRateMultiplier;

	// These Acceleration curves describe the rate at which the camera rotation 
	// speed increases to the Max Velocity at a given input (-1 to 1)
	UPROPERTY(EditDefaultsOnly, Category = "Curve|No Base", meta = (EditCondition = "!bUseBaseVelocity"))
	UCurveVector* MouseLookAccelerationCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Curve|No Base", meta = (EditCondition = "!bUseBaseVelocity"))
	UCurveVector* MouseLookAccelerationADSCurve;

	float GetFOVScaling(ARHPlayerController* pPlayerController) const;

    UPROPERTY(EditDefaultsOnly, Category = "Curve|BaseVelocity", AdvancedDisplay, meta = (EditCondition = "bUseBaseVelocity"))
    TMap<FName, UCurveVector*> TestBaseVelocityCurves;

private:

	UPROPERTY(Transient)
	FVector2D LastRotationVelocityScale;

    UPROPERTY(Transient)
    FVector2D PrevInput;

    UPROPERTY(Transient)
    FVector2D CurrentTurnSpeed;


private:
	static bool StickAimDebug;

public:
	static void ToggleGamepadAimDebug() { StickAimDebug = !StickAimDebug; }
};
