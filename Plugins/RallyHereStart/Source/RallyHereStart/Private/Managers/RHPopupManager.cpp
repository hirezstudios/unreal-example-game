#include "RallyHereStart.h"
#include "Managers/RHPopupManager.h"

URHPopupManager::URHPopupManager(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    m_nPopupId = 0;
    bUsesPopupQueue = false;
    bNewPopupsShowOverCurrent = true;
}

int32 URHPopupManager::AddPopup(const FRHPopupConfig &popupData)
{
    if (bUsesPopupQueue)
    {
        // Return the current popup to the queue, to open the new one over the top
        if (bNewPopupsShowOverCurrent && CurrentPopup.IsValid() && CurrentPopup.CanBeShownOver)
        {
            PopupQueue.Add(CurrentPopup);
            CurrentPopup.PopupId = INDEX_NONE;
        }

        PopupQueue.Push(popupData);
        PopupQueue[PopupQueue.Num() - 1].PopupId = ++m_nPopupId;
        NextPopup();
    }
    else
    {
        // Always assign a new PopupId to each popup so we can compare them if we want to remove specific popups
        if (bNewPopupsShowOverCurrent && (!CurrentPopup.IsValid() || CurrentPopup.CanBeShownOver))
        {
            CurrentPopup = popupData;
            CurrentPopup.PopupId = ++m_nPopupId;

            SetUsesBlocker(CurrentPopup.TreatAsBlocker);
            ShowPopup(CurrentPopup);
            OnShowPopup.Broadcast();
        }
        else
        {
            // Nothing happens as we are not using a queue and not showing over the top.
        }
    }

    return m_nPopupId;
}

void URHPopupManager::RemovePopup(int32 popupId)
{
    if (CurrentPopup.PopupId == popupId)
    {
        OnPopupCanceled();
    }
    else
    {
        for (int32 i = 0; i < PopupQueue.Num(); i++)
        {
            if (PopupQueue[i].PopupId == popupId)
            {
                PopupQueue.RemoveAt(i);
                break;
            }
        }
    }
}

void URHPopupManager::NextPopup()
{
    if (bUsesPopupQueue)
    {
        if (!CurrentPopup.IsValid() && PopupQueue.Num() > 0)
        {
		    CurrentPopup = PopupQueue[PopupQueue.Num() - 1];
		    SetUsesBlocker(CurrentPopup.TreatAsBlocker);
		    ShowPopup(CurrentPopup);
		    OnShowPopup.Broadcast();

            PopupQueue.RemoveAt(PopupQueue.Num() - 1);
	    }
    }
}

void URHPopupManager::CloseAllPopups()
{
    if (CurrentPopup.IsValid())
    {
        HidePopup();
        CurrentPopup.PopupId = INDEX_NONE;
        CurrentPopup.CancelAction.Broadcast();
    }

	PopupQueue.Empty();
	SetUsesBlocker(false);
}

void URHPopupManager::OnPopupCanceled()
{
    if (CurrentPopup.IsValid())
    {
        HidePopup();
        CurrentPopup.PopupId = INDEX_NONE;
        CurrentPopup.CancelAction.Broadcast();
        SetUsesBlocker(false);
        NextPopup();
    }
}

void URHPopupManager::OnPopupResponse(int32 nPopupId, int32 nResponseIndex)
{
    if (CurrentPopup.PopupId == nPopupId)
    {
        if (nResponseIndex < CurrentPopup.Buttons.Num())
        {
            CurrentPopup.Buttons[nResponseIndex].Action.Broadcast();

            // If after the action we are still the same popup, close ourselves
            if (CurrentPopup.PopupId == nPopupId)
            {
                HidePopup();
                CurrentPopup.PopupId = INDEX_NONE;
                NextPopup();
            }
        }
    }
}

void URHPopupManager::CloseUnimportantPopups()
{
    for (int32 i = 0; i < PopupQueue.Num(); i++)
    {
        if (!PopupQueue[i].IsImportant)
        {
            PopupQueue.RemoveAt(i);
            i--;
        }
    }

    if (CurrentPopup.IsValid() && !CurrentPopup.IsImportant)
    {
        OnPopupCanceled();
    }
}

FText URHPopupManager::CommittedTextHandoff()
{
    FText temp = CommittedText;
    CommittedText = FText::FromString(FString());
    return temp;
}