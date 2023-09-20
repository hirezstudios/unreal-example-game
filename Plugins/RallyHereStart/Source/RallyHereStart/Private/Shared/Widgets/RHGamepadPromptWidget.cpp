// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Widgets/RHGamepadPromptWidget.h"

void URHGamepadPromptWidget::PushContext(FButtonPromptContext PromptContext)
{
	ContextStack.Push(PromptContext);
	ApplyContext(ContextStack.Top());
}

bool URHGamepadPromptWidget::PopContext(FButtonPromptContext& OutContext)
{
	if (ContextStack.Num() == 0)
	{
		return false;
	}

	OutContext = ContextStack.Pop();

	if (ContextStack.Num() != 0)
	{
		ApplyContext(ContextStack.Top());
	}
	return true;
}

void URHGamepadPromptWidget::ClearAllContext()
{
	ContextStack.Empty();
}

void URHGamepadPromptWidget::SetContext(FButtonPromptContext PromptContext)
{
	if (ContextStack.Num() == 0)
	{
		PushContext(PromptContext);
	}
	else
	{
		ContextStack[ContextStack.Num() - 1] = PromptContext;
		ApplyContext(ContextStack.Top());
	}
}
