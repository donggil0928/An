#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueChoiceButton.generated.h"

UCLASS()
class UDialogueChoiceButton : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class ANpcActor* OwnerNpc;

	int32 ChoiceIndex = 0;

	UFUNCTION()
	void OnClicked();
};
