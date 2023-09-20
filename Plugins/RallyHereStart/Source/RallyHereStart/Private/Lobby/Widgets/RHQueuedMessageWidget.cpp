// Copyright 2016-2017 Hi-Rez Studios, Inc. All Rights Reserved.

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
