#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

UCLASS()
class ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void SetLoadingProgress(float InProgress);

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	class UWidgetAnimation* PlayAnim;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UProgressBar* ProgressBar_Loading;
};