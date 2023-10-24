#include "RallyHereStart.h"
#include "Managers/RHInputManager.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "Shared/Widgets/RHContextBar.h"

URHContextBar::URHContextBar(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void URHContextBar::InitializeWidget_Implementation()
{
	Super::InitializeWidget_Implementation();

	if (MyHud.IsValid())
	{
		if (URHInputManager* InputManager = MyHud->GetInputManager())
		{
			InputManager->OnContextActionsUpdated.BindDynamic(this, &URHContextBar::RefreshContextBar);
		}
	}
}

void URHContextBar::RefreshContextBar()
{
	if (MyHud.IsValid())
	{
		if (URHInputManager* InputManager = MyHud->GetInputManager())
		{
			TArray<UContextActionData*> TopRouteActions;
			TArray<UContextActionData*> GlobalActions;
			FGameplayTag ActiveRouteTag = FGameplayTag::EmptyTag; //$$ KAB - Route names changed to Gameplay Tags

			if (InputManager->GetActiveContextActions(TopRouteActions, GlobalActions, ActiveRouteTag)) //$$ KAB - Route names changed to Gameplay Tags
			{
				UpdateContextActions(TopRouteActions, GlobalActions, ActiveRouteTag); //$$ KAB - Route names changed to Gameplay Tags
			}
			else
			{
				UpdateContextActions(TArray<UContextActionData*>(), TArray<UContextActionData*>(), FGameplayTag::EmptyTag); //$$ KAB - Route names changed to Gameplay Tags
			}
		}
	}
}

void URHContextBar::UpdateContextActions(const TArray<UContextActionData*>& RouteActions, const TArray<UContextActionData*>& GlobalActions, const FGameplayTag& ActiveRouteTag) //$$ KAB - Route names changed to Gameplay Tags
{
	// Build out a list of actions we are looking to add via the RouteActions/GLobalActions
	TArray<FName> AddList;

	for (UContextActionData* Data : RouteActions)
	{
		if (Data)
		{
			AddList.Add(Data->RowName);
		}
	}

	for (UContextActionData* Data : GlobalActions)
	{
		if (Data)
		{
			AddList.Add(Data->RowName);
		}
	}

	// First remove all the current children of the containers to the pool, then rebuild the widgets
	ReturnContainerContentsToPool(LeftContainer, AddList, RouteActions);
	ReturnContainerContentsToPool(CenterContainer, AddList, RouteActions);
	ReturnContainerContentsToPool(RightContainer, AddList, RouteActions);

	// Add context actions to the bar
	AddContextActions(RouteActions, AddList);
	AddContextActions(GlobalActions, AddList);

	bool bAlwaysShowContextBar = false;
	if (MyHud != nullptr && MyHud->GetViewManager() && ActiveRouteTag.IsValid()) //$$ KAB - Route names changed to Gameplay Tags
	{
		FViewRoute ViewRoute;
		if (MyHud->GetViewManager()->GetViewRoute(ActiveRouteTag, ViewRoute))
		{
			bAlwaysShowContextBar = ViewRoute.AlwaysShowContextBar;
		}
	}

	if (HasContextActions())
	{
		ForceLayoutPrepass();
	}

	// Set the visibility of the context bar based on if we have any context actions
	SetVisibility((bAlwaysShowContextBar || HasContextActions()) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void URHContextBar::AddContextActions(const TArray<UContextActionData*>& RouteActions, TArray<FName>& AddList)
{
	for (UContextActionData* ActionData : RouteActions)
	{
		if (ActionData && !ActionData->RowData.IsHidden && AddList.Contains(ActionData->RowName))
		{
			URHContextBarPrompt* WidgetToAdd = nullptr;

			if (ActionData->RowData.PromptWidget != nullptr)
			{
				// Create the custom widget being added
				WidgetToAdd = Cast<URHContextBarPrompt>(CreateWidget(this, ActionData->RowData.PromptWidget));
				WidgetToAdd->InitializeWidget();
				// Explicitly set custom widgets to not being poolable
				WidgetToAdd->SetPoolability(false);
			}
			else
			{
				// Check the pool for an available widget
				if (PromptPool.Num())
				{
					WidgetToAdd = PromptPool.Pop();
				}
				else if (PromptWidgetClass != nullptr)
				{
					WidgetToAdd = Cast<URHContextBarPrompt>(CreateWidget(this, PromptWidgetClass));
					WidgetToAdd->InitializeWidget();
					// Regularly created widgets should be poolable by default
					WidgetToAdd->SetPoolability(true);
				}
			}

			if (WidgetToAdd)
			{
				WidgetToAdd->SetActive(true);
				WidgetToAdd->SetContextActionData(ActionData);

				switch (ActionData->RowData.Anchor)
				{
					case EContextPromptAnchoring::AnchorLeft:
					{
						UHorizontalBoxSlot* ContextSlot = LeftContainer->AddChildToHorizontalBox(WidgetToAdd);
						ContextSlot->SetPadding(LeftPromptMargin);
						ContextSlot->SetVerticalAlignment(VAlign_Center);
						break;
					}
					case EContextPromptAnchoring::AnchorRight:
					{
						UHorizontalBoxSlot* ContextSlot = RightContainer->AddChildToHorizontalBox(WidgetToAdd);
						ContextSlot->SetPadding(RightPromptMargin);
						ContextSlot->SetVerticalAlignment(VAlign_Center);
						break;
					}
					case EContextPromptAnchoring::AnchorCenter:
					{
						UHorizontalBoxSlot* ContextSlot = CenterContainer->AddChildToHorizontalBox(WidgetToAdd);
						ContextSlot->SetPadding(CenterPromptMargin);
						ContextSlot->SetVerticalAlignment(VAlign_Center);
						break;
					}
				}
			}
		}
	}
}

void URHContextBar::ReturnContainerContentsToPool(UHorizontalBox* Container, TArray<FName>& AddList, const TArray<UContextActionData*>& RouteActions)
{
	TArray<UWidget*> WidgetsToRemove;

	for (UWidget* Child : Container->GetAllChildren())
	{
		if (URHContextBarPrompt* PromptWidget = Cast<URHContextBarPrompt>(Child))
		{
			UContextActionData* Data = PromptWidget->GetContextActionData();

			// If we are going to add this prompt, leave it in the container
			if (Data && AddList.Contains(Data->RowName))
			{
				// Remove it from the add list as we don't need to add this widget anymore
				AddList.Remove(Data->RowName);

				// Update the data on the prompt as it may have same name but different callbacks
				for (UContextActionData* RouteData : RouteActions)
				{
					if (Data && RouteData->RowName == Data->RowName)
					{
						PromptWidget->SetContextActionData(RouteData);
						break;
					}
				}

				continue;
			}

			// Only return the base prompts to the pool, we don't want other widget types in it
			if (PromptWidget->CanBePooled())
			{
				PromptWidget->SetActive(false);
				PromptWidget->SetContextActionData(nullptr);
				PromptPool.Add(PromptWidget);
			}

			WidgetsToRemove.Add(PromptWidget);
		}
	}

	for (UWidget* Widget : WidgetsToRemove)
	{
		Container->RemoveChild(Widget);
	}
}

bool URHContextBar::HasContextActions()
{
	RH_INPUT_STATE InputState = MyHud->GetCurrentInputState();
	return HasContextActionForInput(LeftContainer, InputState) || HasContextActionForInput(RightContainer, InputState) || HasContextActionForInput(CenterContainer, InputState);
}

bool URHContextBar::HasContextActionForInput(UHorizontalBox* Container, RH_INPUT_STATE InputState)
{
	for (UWidget* Child : Container->GetAllChildren())
	{
		if (URHContextBarPrompt* PromptWidget = Cast< URHContextBarPrompt>(Child))
		{
			if (PromptWidget->IsValidForInput(InputState))
			{
				return true;
			}
		}
	}

	return false;
}