#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractHintWidget.generated.h"

UCLASS()
class UInteractHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Hint")
	void SetHintText(const FString& ActionName);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* Text_Action;
};