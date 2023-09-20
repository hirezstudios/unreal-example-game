// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "TickAnimationManager.h"

UTickAnimationManager::UTickAnimationManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UTickAnimationManager::ApplyTick(float DeltaTime)
{
	for (auto AnimKV : AnimsByName)
	{
		FTickAnimationParams& Anim = AnimsByName[AnimKV.Key];
		if (Anim.IsPlaying)
		{
			float NextTime = Anim.ElapsedTime + DeltaTime;
			if (NextTime >= Anim.Duration)
			{
				SkipToEndAnimation(AnimKV.Key);
			}
			else
			{
				Anim.ElapsedTime = NextTime;
				Anim.UpdateEvent.ExecuteIfBound(Anim.ElapsedTime, Anim.ElapsedTime / Anim.Duration);
			}
		}
	}
}

void UTickAnimationManager::AddAnimation(FName AnimName, float Duration, const FTickAnimationUpdate& UpdateEvent, const FTickAnimationFinished& FinishedEvent)
{
	FTickAnimationParams TickAnimParams(Duration, UpdateEvent, FinishedEvent);
	
	AnimsByName.Add(AnimName, TickAnimParams);
}

void UTickAnimationManager::RemoveAnimation(FName AnimName)
{
	if (AnimsByName.Contains(AnimName))
	{
		StopAnimation(AnimName);

		AnimsByName.Remove(AnimName);
	}
}

void UTickAnimationManager::PlayAnimation(FName AnimName)
{
	if (AnimsByName.Contains(AnimName))
	{
		UE_LOG(RallyHereStart, VeryVerbose, TEXT("UTickAnimationManager::PlayAnimation Playing tick animation %s"), *AnimName.ToString());
		FTickAnimationParams& Anim = AnimsByName[AnimName];

		Anim.IsPlaying = true;
		Anim.ElapsedTime = 0.0f;
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("UTickAnimationManager::PlayAnimation animation %s not found."), *AnimName.ToString());
	}
}

void UTickAnimationManager::StopAnimation(FName AnimName)
{
	if (AnimsByName.Contains(AnimName))
	{
		UE_LOG(RallyHereStart, VeryVerbose, TEXT("UTickAnimationManager::StopAnimation Stopping tick animation %s"), *AnimName.ToString());
		FTickAnimationParams& Anim = AnimsByName[AnimName];

		Anim.IsPlaying = false;
		Anim.ElapsedTime = 0.0f;
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("UTickAnimationManager::StopAnimation animation %s not found."), *AnimName.ToString());
	}
}

void UTickAnimationManager::PauseAnimation(FName AnimName)
{
	if (AnimsByName.Contains(AnimName))
	{
		UE_LOG(RallyHereStart, VeryVerbose, TEXT("UTickAnimationManager::PauseAnimation Pausing tick animation %s"), *AnimName.ToString());
		FTickAnimationParams& Anim = AnimsByName[AnimName];
		Anim.IsPlaying = false;
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("UTickAnimationManager::PauseAnimation animation %s not found."), *AnimName.ToString());
	}
}

void UTickAnimationManager::ResumeAnimation(FName AnimName)
{
	if (AnimsByName.Contains(AnimName))
	{
		UE_LOG(RallyHereStart, VeryVerbose, TEXT("UTickAnimationManager::ResumeAnimation Resuming tick animation %s"), *AnimName.ToString());
		FTickAnimationParams& Anim = AnimsByName[AnimName];
		Anim.IsPlaying = true;
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("UTickAnimationManager::ResumeAnimation animation %s not found."), *AnimName.ToString());
	}
}

void UTickAnimationManager::SkipToEndAnimation(FName AnimName)
{
	if (AnimsByName.Contains(AnimName))
	{
		UE_LOG(RallyHereStart, VeryVerbose, TEXT("UTickAnimationManager::SkipToEndAnimation Skipping to end of animation %s"), *AnimName.ToString());
		FTickAnimationParams& Anim = AnimsByName[AnimName];

		Anim.IsPlaying = false;
		Anim.ElapsedTime = Anim.Duration;

		Anim.UpdateEvent.ExecuteIfBound(Anim.ElapsedTime, Anim.ElapsedTime / Anim.Duration);
		Anim.FinishedEvent.ExecuteIfBound();
	}
	else
	{
		UE_LOG(RallyHereStart, Warning, TEXT("UTickAnimationManager::SkipToEndAnimation animation %s not found."), *AnimName.ToString());
	}
}

bool UTickAnimationManager::GetAnimationInfo(FName AnimName, FTickAnimationParams& OutAnimParams)
{
    if (AnimsByName.Num() > 0)
    {
        FTickAnimationParams* AnimParamsPtr = AnimsByName.Find(AnimName);

        if (AnimParamsPtr != nullptr)
        {
            OutAnimParams = *AnimParamsPtr;
            return true;
        }
    }

	return false;
}
