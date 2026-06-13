#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"


UCLASS()
class ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	class UWidgetAnimation* PlayAnim;
};
