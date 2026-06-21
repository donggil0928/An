#include "InteractHintWidget.h"
#include "Components/TextBlock.h"

void UInteractHintWidget::SetHintText(const FString& ActionName)
{
	if (Text_Action)
		Text_Action->SetText(FText::FromString(ActionName));
}