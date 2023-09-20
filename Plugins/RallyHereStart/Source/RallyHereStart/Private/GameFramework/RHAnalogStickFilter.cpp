// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "GameFramework/RHAnalogStickFilter.h"

URHAnalogStickFilter::URHAnalogStickFilter(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	StickType = EAnalogStickType::Unknown;
	PlayerInput = nullptr;
}

FVector2D URHAnalogStickFilter::FilterStickInput(FVector2D RawStickInput)
{
	return RawStickInput;
}
