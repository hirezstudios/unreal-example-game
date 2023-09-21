// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Lobby/Widgets/RHQueuedMessageWidget.h"

void URHQueuedMessageWidget::QueueMessage(FText Message)
{
    MessageQueue.Enqueue(Message);
}

bool URHQueuedMessageWidget::GetNextMessage(FText& Message)
{
    return MessageQueue.Dequeue(Message);
}
