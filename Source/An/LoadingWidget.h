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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	class UWidgetAnimation* PlayAnim;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UImage* Image_Lamp;

	UPROPERTY()
	class UMaterialInstanceDynamic* LampMaterialInstance;

private:
	float TargetProgress = 0.f;
	float CurrentProgress = 0.f;
};