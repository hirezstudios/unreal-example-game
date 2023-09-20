// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "GameFramework/RHGamepadCurvedLookSpeedManager.h"
#include "Player/Controllers/RHPlayerController.h"
#include "GameFramework/RHPlayerInput.h"
#include "Runtime/Engine/Classes/Curves/CurveVector.h"
#include "GameFramework/InputSettings.h"
#include "Camera/PlayerCameraManager.h"

#define LOOK_ACCELERATION_EVAL_RATE 150.f
#define LOW_FPS_LOOK_ACCELERATION_EVALS_PER_FRAME 10.f

bool URHGamepadCurvedLookSpeedManager::StickAimDebug = false;

static int32 GEnableAimAcceleration = 1;
static FAutoConsoleVariableRef CVarEnableAimAcceleration(
	TEXT("game.AimAcceleration"),
	GEnableAimAcceleration,
	TEXT(" 0: Disable gamepad aim acceleration\n")
	TEXT(" 1: Enable gamepad aim acceleration(default)"),
	ECVF_Default
);

URHGamepadCurvedLookSpeedManager::URHGamepadCurvedLookSpeedManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bUseBaseVelocity = true;

    static ConstructorHelpers::FObjectFinder<UCurveVector> BaseVelCurveFinder(TEXT("/RallyHereStart/GameFramework/AnalogStickFilters/LookSpeedBaseVelocity_Curve.LookSpeedBaseVelocity_Curve"));
    LookBaseVelocityCurve = BaseVelCurveFinder.Object;

    BoostThreshold.X = 0.959f;
    BoostThreshold.Y = 0.959f;

    BoostMultiplier.X = 1.2f;
    BoostMultiplier.Y = 1.2f;

    BoostAcceleration = .45f;

	// Velocity
	static ConstructorHelpers::FObjectFinder<UCurveVector> VCurveFinder(TEXT("/RallyHereStart/GameFramework/AnalogStickFilters/LookSpeedMaxVelocity_Curve.LookSpeedMaxVelocity_Curve"));
	MouseLookMaxVelocityCurve = VCurveFinder.Object;

	// ADS Velocity
	static ConstructorHelpers::FObjectFinder<UCurveVector> ADSCurveFinder(TEXT("/RallyHereStart/GameFramework/AnalogStickFilters/LookSpeedMaxVelocity_Curve.LookSpeedMaxVelocity_Curve"));
	MouseLookMaxVelocityADSCurve = ADSCurveFinder.Object;

	// Acceleration
	static ConstructorHelpers::FObjectFinder<UCurveVector> AccCurveFinder(TEXT("/RallyHereStart/GameFramework/AnalogStickFilters/LookSpeedAcceleration_Curve.LookSpeedAcceleration_Curve"));
	MouseLookAccelerationCurve = AccCurveFinder.Object;

	// ADS Acceleration
	static ConstructorHelpers::FObjectFinder<UCurveVector> ADSAccCurveFinder(TEXT("/RallyHereStart/GameFramework/AnalogStickFilters/LookSpeedAcceleration_Curve.LookSpeedAcceleration_Curve"));
	MouseLookAccelerationADSCurve = ADSAccCurveFinder.Object;

	BaseLookRateScale = FVector2D(50.0f, 50.0f);
	TurnRateMultiplier = 60.0f;
	LookUpRateMultiplier = 60.0f;
	LastRotationVelocityScale = FVector2D::ZeroVector;

    CurrentTurnSpeed = FVector2D::ZeroVector;
    PrevInput = FVector2D::ZeroVector;
}

void URHGamepadCurvedLookSpeedManager::Init()
{
	Super::Init();

	if (PlayerController != nullptr)
	{
		if (URHPlayerInput* const playerInput = Cast<URHPlayerInput>(PlayerController->PlayerInput))
		{
			float OutValue = BoostAcceleration;
			if (playerInput->GetSettingAsFloat(URHPlayerInput::GamepadAccelerationBoost, OutValue))
			{
				SetBoostAcceleration(OutValue);
			}
			if (playerInput->GetSettingAsFloat(URHPlayerInput::GamepadMultiplierBoost, OutValue))
			{
				SetBoostMultiplier(OutValue, OutValue);
			}
		}
	}
}

FVector2D URHGamepadCurvedLookSpeedManager::UpdateGamepadLook(FVector2D GamepadInput, float DeltaTime)
{
	// TODO: If we have any UI open where the right stick is being used for another purpose, return ZeroVector for look
/*	if (PlayerController && PlayerController->HasUIBlockingRightStickLookInput())
	{
		return FVector2D::ZeroVector;
	}
*/

	FVector2D LookChangeThisFrame = GamepadInput;
	if (GEnableAimAcceleration && PlayerController != nullptr)
	{
		if (const URHPlayerInput* playerInput = Cast<URHPlayerInput>(PlayerController->PlayerInput))
		{
            // Realm method + FOV scaling and sensitivity

            //We want to use a fixed timestep of 1/150 sec, but at low framerates, in order to still maintain smooth acceleration calculations,
            //we switch to doing a defined amount per frame.
            const float AccelerationTimestep = FMath::Max(DeltaTime / LOW_FPS_LOOK_ACCELERATION_EVALS_PER_FRAME, 1.f / LOOK_ACCELERATION_EVAL_RATE);

			const FVector2D sensitivity = playerInput->GetGamepadLookSensitivity();
	
			// TODO: Hook up Scope/ADS sensitivity modifiers
/*			switch (playerInput->AimMode)
			{
			case EHRCharacterAimMode::AimDownSights:
				sensitivity *= playerInput->GetGamepadLookSensitivityScope() / 100.f;
				break;
			case ERHCharacterAimMode::Shoulder:
				sensitivity *= playerInput->GetGamepadLookSensitivityADS() / 100.f;
				break;
			}
*/
			float aTurn = GamepadInput.X, aLookUp = GamepadInput.Y;

			const bool bZeroTurn = FMath::IsNearlyZero(aTurn, KINDA_SMALL_NUMBER);
			const bool bZeroLookUp = FMath::IsNearlyZero(aLookUp, KINDA_SMALL_NUMBER);

			const float dirX = (aTurn > 0.0f ? 1.0f : -1.0f);
            const float dirY = (aLookUp > 0.0f ? 1.0f : -1.0f);

            FVector2D AverageTurnSpeed = FVector2D::ZeroVector;
            float AccelerationTimeRemaining = DeltaTime;
            FVector2D acceleration = FVector2D::ZeroVector;

            if (bUseBaseVelocity)
            {
                while (AccelerationTimeRemaining > 0.0005f)
                {
                    const float Timestep = FMath::Min(AccelerationTimestep, AccelerationTimeRemaining);

                    if (!bZeroTurn)
                    {
                        const float BaseTurnSpeed = LookBaseVelocityCurve->FloatCurves[0].Eval(FMath::Abs(aTurn));

                        if (FMath::Abs(aTurn) < BoostThreshold.X || FMath::Abs(CurrentTurnSpeed.X) < BaseTurnSpeed || FMath::Sign(CurrentTurnSpeed.X) != dirX)
                        {
                            //Set the turn speed directly if we aren't at a boostable input or the current speed isn't yet at base.
                            CurrentTurnSpeed.X = BaseTurnSpeed * dirX;
                        }

                        if (FMath::Abs(aTurn) >= BoostThreshold.X)
                        {
                            acceleration.X = BoostAcceleration;
                            CurrentTurnSpeed.X += Timestep * dirX * acceleration.X;

                            const float DesiredTurnSpeed = BaseTurnSpeed * BoostMultiplier.X;
                            CurrentTurnSpeed.X = FMath::Clamp(CurrentTurnSpeed.X, -DesiredTurnSpeed, DesiredTurnSpeed);
                        }
						
                        AverageTurnSpeed.X += CurrentTurnSpeed.X * Timestep;
                    }

                    if (!bZeroLookUp)
                    {
                        const float BaseLookUpSpeed = LookBaseVelocityCurve->FloatCurves[1].Eval(FMath::Abs(aLookUp));

                        if (FMath::Abs(aLookUp) < BoostThreshold.Y || FMath::Abs(CurrentTurnSpeed.Y) < BaseLookUpSpeed || FMath::Sign(CurrentTurnSpeed.Y) != dirY)
                        {
                            //Set the lookup speed directly if we aren't at a boostable input or the current speed isn't yet at base.
                            CurrentTurnSpeed.Y = BaseLookUpSpeed * dirY;
                        }

                        if (FMath::Abs(aLookUp) >= BoostThreshold.Y)
                        {
                            acceleration.Y = BoostAcceleration;
                            CurrentTurnSpeed.Y += Timestep * dirY * acceleration.Y;

                            const float DesiredLookUpSpeed = BaseLookUpSpeed * BoostMultiplier.Y;
                            CurrentTurnSpeed.Y = FMath::Clamp(CurrentTurnSpeed.Y, -DesiredLookUpSpeed, DesiredLookUpSpeed);
                        }

                        AverageTurnSpeed.Y += CurrentTurnSpeed.Y * Timestep;
                    }

                    AccelerationTimeRemaining -= Timestep;
                }
            }
            else if (MouseLookAccelerationCurve != nullptr)
            {
                if (CurrentTurnSpeed.X * dirX <= 0 || bZeroTurn)
                {
                    PrevInput.X = 0.f;
                    CurrentTurnSpeed.X = 0.f;
                }
                if (CurrentTurnSpeed.Y * dirY <= 0 || bZeroLookUp)
                {
                    PrevInput.Y = 0.f;
                    CurrentTurnSpeed.Y = 0.f;
                }

                const FVector2D OldInput = PrevInput;

                while (AccelerationTimeRemaining > 0.0005f)
                {
                    const float Timestep = FMath::Min(AccelerationTimestep, AccelerationTimeRemaining);

                    if (!bZeroTurn)
                    {
                        const float StepATurn = FMath::Lerp(OldInput.X, aTurn, 1.f - (AccelerationTimeRemaining / DeltaTime));
                        const float DesiredTurnSpeed = MouseLookMaxVelocityCurve->FloatCurves[0].Eval(FMath::Abs(StepATurn));
                        acceleration.X = MouseLookAccelerationCurve->FloatCurves[0].Eval(FMath::Abs(CurrentTurnSpeed.X));

                        CurrentTurnSpeed.X += Timestep * dirX * acceleration.X;
                        CurrentTurnSpeed.X = FMath::Clamp(CurrentTurnSpeed.X, -DesiredTurnSpeed, DesiredTurnSpeed);
						
                        AverageTurnSpeed.X += CurrentTurnSpeed.X * Timestep;
                    }

                    if (!bZeroLookUp)
                    {
                        const float StepALookUp = FMath::Lerp(OldInput.Y, aLookUp, 1.f - (AccelerationTimeRemaining / DeltaTime));
                        const float DesiredTurnSpeed = MouseLookMaxVelocityCurve->FloatCurves[1].Eval(FMath::Abs(StepALookUp));

                        acceleration.Y = MouseLookAccelerationCurve->FloatCurves[1].Eval(FMath::Abs(CurrentTurnSpeed.Y));

                        CurrentTurnSpeed.Y += Timestep * dirY * acceleration.Y;
                        CurrentTurnSpeed.Y = FMath::Clamp(CurrentTurnSpeed.Y, -DesiredTurnSpeed, DesiredTurnSpeed);

                        AverageTurnSpeed.Y += CurrentTurnSpeed.Y * Timestep;
                    }

                    AccelerationTimeRemaining -= Timestep;
                }
            }

			if (DeltaTime > 0.f)
			{
				AverageTurnSpeed.X /= DeltaTime;
				AverageTurnSpeed.Y /= DeltaTime;
			}
			else
			{
				AverageTurnSpeed.X = CurrentTurnSpeed.X;
				AverageTurnSpeed.Y = CurrentTurnSpeed.Y;
			}

			PrevInput.X = bZeroTurn ? 0.f : aTurn;
			PrevInput.Y = bZeroLookUp ? 0.f : aLookUp;

			// Ensure that no input is always treated as no input
			aTurn = bZeroTurn ? 0.f : AverageTurnSpeed.X * BaseLookRateScale.X;
			aLookUp = bZeroLookUp ? 0.f : AverageTurnSpeed.Y * BaseLookRateScale.Y;

			LookChangeThisFrame = FVector2D(aTurn * sensitivity.X * DeltaTime, aLookUp * sensitivity.Y * DeltaTime);
			FVector2D vel = FVector2D(aTurn * sensitivity.X, aLookUp * sensitivity.Y);

			if (URHGamepadCurvedLookSpeedManager::StickAimDebug)
			{
				GEngine->AddOnScreenDebugMessage(1, .2f, FColor::Emerald, FString(TEXT("Input: ") + GamepadInput.ToString()));
				GEngine->AddOnScreenDebugMessage(2, .2f, FColor::Emerald, FString(TEXT("Acceleration: ") + acceleration.ToString()));
				GEngine->AddOnScreenDebugMessage(3, .2f, FColor::Emerald, FString(TEXT("Output Velocity: ") + vel.ToString()));
				GEngine->AddOnScreenDebugMessage(4, .2f, FColor::Emerald, FString(TEXT("Output Look Change: ") + LookChangeThisFrame.ToString()));
			}
		}
	}
	else if (PlayerController != nullptr)
	{
		if (const URHPlayerInput* playerInput = Cast<URHPlayerInput>(PlayerController->PlayerInput))
		{
			const FVector2D sensitivity = playerInput->GetGamepadLookSensitivity();
			
			// TODO: Hook up Scope/ADS sensitivity modifiers
/*			switch (playerInput->AimMode)
			{
			case ERHCharacterAimMode::AimDownSights:
				sensitivity *= playerInput->GetGamepadLookSensitivityScope() / 100.f;
				break;
			case ERHCharacterAimMode::Shoulder:
				sensitivity *= playerInput->GetGamepadLookSensitivityADS() / 100.f;
				break;
			}
*/
			LookChangeThisFrame = LookChangeThisFrame * BaseLookRateScale * sensitivity * DeltaTime;
		}
	}

	return LookChangeThisFrame;
}

float URHGamepadCurvedLookSpeedManager::GetFOVScaling(ARHPlayerController* pPlayerController) const
{
	const UInputSettings* const DefaultInputSettings = GetDefault<UInputSettings>();
	check(DefaultInputSettings != nullptr);
	if (DefaultInputSettings->bEnableFOVScaling)
	{
		if (pPlayerController != nullptr && pPlayerController->PlayerCameraManager != nullptr)
		{
			return (DefaultInputSettings->FOVScale) * (pPlayerController->PlayerCameraManager->GetFOVAngle());
		}
	}
	return 1.0f;
}

void URHGamepadCurvedLookSpeedManager::SetOuterDeadZone(float NewZoneX, float NewZoneY)
{
    if (NewZoneX > 0.f && NewZoneX <= 1.f)
    {
        BoostThreshold.X = NewZoneX;
    }

    if (NewZoneY > 0.f && NewZoneY <= 1.f)
    {
        BoostThreshold.Y = NewZoneY;
    }
}

void URHGamepadCurvedLookSpeedManager::SetBoostMultiplier(float NewBoostX, float NewBoostY)
{
    if (NewBoostX > 0.f)
    {
        BoostMultiplier.X = NewBoostX;
    }

    if (NewBoostY > 0.f)
    {
        BoostMultiplier.Y = NewBoostY;
    }
}

void URHGamepadCurvedLookSpeedManager::SetBoostAcceleration(float NewAccel)
{
    if (NewAccel > 0.f)
    {
        BoostAcceleration = NewAccel;
    }
}

void URHGamepadCurvedLookSpeedManager::SetBaseVelocityCurve(FName TestCurveName)
{
    if (TestCurveName != NAME_None)
    {
        UCurveVector** TestCurve = TestBaseVelocityCurves.Find(TestCurveName);
        if (TestCurve != nullptr && *TestCurve != nullptr)
        {
            LookBaseVelocityCurve = *TestCurve;
        }
    }
}

void URHGamepadCurvedLookSpeedManager::PrintLookSpeedParameters()
{
    if (PlayerController != nullptr)
    {
        const FString BaseTurnString = FString::Printf(TEXT("Base turn rate: yaw = %f; pitch = %f"), BaseLookRateScale.X, BaseLookRateScale.Y);
        PlayerController->ClientMessage(BaseTurnString);

		if (URHPlayerInput* const playerInput = Cast<URHPlayerInput>(PlayerController->PlayerInput))
		{
            const FVector2D Sensitivity = playerInput->GetGamepadLookSensitivity();

            const FString SensitivityString = FString::Printf(TEXT("Current sensitivity: yaw = %f; pitch = %f"), Sensitivity.X, Sensitivity.Y);
            PlayerController->ClientMessage(SensitivityString);
        }
        else
        {
            PlayerController->ClientMessage(TEXT("Unknown sensitivity settings due to invalid Player Input object!!!"));
        }

        if (LookBaseVelocityCurve != nullptr)
        {
            const FString CurveString = FString::Printf(TEXT("Look response curve: %s"), *LookBaseVelocityCurve->GetName());
            PlayerController->ClientMessage(CurveString);
        }
        else
        {
            PlayerController->ClientMessage(TEXT("No look response curve!!!"));
        }

        const FString OuterDeadZoneString = FString::Printf(TEXT("Outer dead zone: yaw = %f, pitch = %f"), BoostThreshold.X, BoostThreshold.Y);
        PlayerController->ClientMessage(OuterDeadZoneString);

        const FString BoostString = FString::Printf(TEXT("Boost multiplier: yaw = %f, pitch = %f"), BoostMultiplier.X, BoostMultiplier.Y);
        PlayerController->ClientMessage(BoostString);

        const FString AccelString = FString::Printf(TEXT("Boost acceleration: %f"), BoostAcceleration);
        PlayerController->ClientMessage(AccelString);
    }
    else
    {
        UE_LOG(RallyHereStart, Error, TEXT("No player controller to print look speed params!"));
    }
}
