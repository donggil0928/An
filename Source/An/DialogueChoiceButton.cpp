#include "DialogueChoiceButton.h"
#include "NpcActor.h"

void UDialogueChoiceButton::OnClicked()
{
	if (OwnerNpc)
		OwnerNpc->SelectChoice(ChoiceIndex);
}
