// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "GameFramework/RHBoxDeadZoneFilter.h"

URHBoxDeadZoneFilter::URHBoxDeadZoneFilter(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	InnerDeadZone = FVector2D(0.1f, 0.1f);
	OuterDeadZone = FVector2D(0.12f, 0.12f);
}

FVector2D URHBoxDeadZoneFilter::FilterStickInput(FVector2D RawStickInput)
{
	FVector2D FilteredDeadZoneInput;

	FVector2D InvOuterDeadZone = FVector2D(1.f, 1.f) - OuterDeadZone;

	FilteredDeadZoneInput.X = FMath::Sign(RawStickInput.X) * FMath::GetMappedRangeValueClamped(FVector2D(InnerDeadZone.X, InvOuterDeadZone.X), FVector2D(0.f, 1.f), FMath::Abs(RawStickInput.X));
	FilteredDeadZoneInput.Y = FMath::Sign(RawStickInput.Y) * FMath::GetMappedRangeValueClamped(FVector2D(InnerDeadZone.Y, InvOuterDeadZone.Y), FVector2D(0.f, 1.f), FMath::Abs(RawStickInput.Y));

	return FilteredDeadZoneInput;
}
