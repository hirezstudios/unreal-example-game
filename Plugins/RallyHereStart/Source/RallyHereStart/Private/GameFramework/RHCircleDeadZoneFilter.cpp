// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "GameFramework/RHCircleDeadZoneFilter.h"
#include "GameFramework/RHPlayerInput.h"

URHCircleDeadZoneFilter::URHCircleDeadZoneFilter(const FObjectInitializer& ObjectInitializer /* = FObjectInitializer::Get() */)
    : Super(ObjectInitializer)
{
    DeadZoneRadius = 0.15f;
    CardinalDeadZoneHalfWidth = 0.05f;
}

FVector2D URHCircleDeadZoneFilter::FilterStickInput(FVector2D RawStickInput)
{
	float DeadZoneSetting = DeadZoneRadius;
	if (PlayerInput != nullptr)
	{
		// Get deadzone radius from the player's settings.
		if (StickType == EAnalogStickType::Left)
		{
			PlayerInput->GetSettingAsFloat(URHPlayerInput::GamepadLeftDeadZone, DeadZoneSetting);
		}
		else if (StickType == EAnalogStickType::Right)
		{
			PlayerInput->GetSettingAsFloat(URHPlayerInput::GamepadRightDeadZone, DeadZoneSetting);
		}
	}

	//The math breaks down if the cardinal dead zone is greater than the radial deadzone.
	const float CardinalDeadZoneSetting = FMath::Min(CardinalDeadZoneHalfWidth, DeadZoneSetting);

    if (FMath::IsNearlyZero(DeadZoneSetting, KINDA_SMALL_NUMBER) || DeadZoneSetting < 0.f)
    {
        return RawStickInput;
    }

    if (RawStickInput.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return FVector2D::ZeroVector;
    }

    const float RawInputSize = RawStickInput.Size();

    if (RawInputSize < DeadZoneSetting)
    {
        return FVector2D::ZeroVector;
    }

    FVector2D CrossFilterInput = RawStickInput;

    if (CardinalDeadZoneSetting > 0.f && CardinalDeadZoneSetting < 1.f)
    {
        if (FMath::Abs(CrossFilterInput.X) < CardinalDeadZoneSetting)
        {
            CrossFilterInput.X = 0.f;
        }
        else
        {
            CrossFilterInput.X = FMath::Sign(CrossFilterInput.X) * (FMath::Abs(CrossFilterInput.X) - CardinalDeadZoneSetting) / (1.f - CardinalDeadZoneSetting);
        }

        if (FMath::Abs(CrossFilterInput.Y) < CardinalDeadZoneSetting)
        {
            CrossFilterInput.Y = 0.f;
        }
        else
        {
            CrossFilterInput.Y = FMath::Sign(CrossFilterInput.Y) * (FMath::Abs(CrossFilterInput.Y) - CardinalDeadZoneSetting) / (1.f - CardinalDeadZoneSetting);
        }
    }

    const FVector2D InputDirection = CrossFilterInput.GetSafeNormal();

    const float FilteredInputSize = (RawInputSize - DeadZoneSetting) / (1.f - DeadZoneSetting);

    return InputDirection * FilteredInputSize;
}
