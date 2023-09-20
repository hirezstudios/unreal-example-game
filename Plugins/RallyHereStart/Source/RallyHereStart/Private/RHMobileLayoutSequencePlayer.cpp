// Copyright 2016-2021 Hi-Rez Studios, Inc. All Rights Reserved.

#include "RHMobileLayoutSequencePlayer.h"
#include "MovieScene.h"
#include "Animation/WidgetAnimation.h"
#include "Evaluation/MovieScenePlayback.h"
#include "MovieSceneTimeHelpers.h"

void URHMobileLayoutSequencePlayer::ActivateMobileLayout(UWidgetAnimation& InAnimation)
{
	UMovieScene* MovieScene = InAnimation.GetMovieScene();

	PreAnimatedState.EnableGlobalPreAnimatedStateCapture();
	GetEvaluationTemplate().Initialize(InAnimation, *this, nullptr, nullptr);

	//Evaluate the animation using the last frame
	if (GetEvaluationTemplate().IsValid())
	{
		if (MovieScene != nullptr)
		{
			// Cache the time range of the sequence to determine when we stop
			int32 LocalDuration = UE::MovieScene::DiscreteSize(MovieScene->GetPlaybackRange());
			FFrameRate LocalAnimationResolution = MovieScene->GetTickResolution();
			FFrameNumber LocalAbsolutePlaybackStart = UE::MovieScene::DiscreteInclusiveLower(MovieScene->GetPlaybackRange());
			FFrameTime LastValidFrame(LocalDuration - 1, 0.99999994f);
			const FMovieSceneContext Context(FMovieSceneEvaluationRange(LocalAbsolutePlaybackStart + LastValidFrame, LocalAbsolutePlaybackStart, LocalAnimationResolution), EMovieScenePlayerStatus::Playing);
			GetEvaluationTemplate().EvaluateSynchronousBlocking(Context, *this);
		}
	}
}

void URHMobileLayoutSequencePlayer::DeactivateMobileLayout()
{
	RestorePreAnimatedState();
}
