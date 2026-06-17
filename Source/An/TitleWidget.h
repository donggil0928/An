#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleWidget.generated.h"


UCLASS()
class UTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Title")
	class ATitleGameMode* OwnerGameMode;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_StartGame;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Continue;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Setting;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Exit;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* Img_Arrow;

	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnHoverStartGame();
	UFUNCTION()
	void OnHoverContinue();
	UFUNCTION()
	void OnHoverSetting();
	UFUNCTION()
	void OnHoverExit();

	UFUNCTION()
	void OnUnhover();

	void SetArrowTargetButton(class UButton* Btn);
	void BindButtonHover(class UButton* Btn, FName HoverFuncName);
	
	UPROPERTY()
	class UButton* CurrentTargetBtn = nullptr;
	
	UPROPERTY()
	class UButton* DefaultBtn = nullptr;
	
	float ArrowInterpSpeed = 12.f;
	
	float ArrowGapX = 20.f;
	
	bool bArrowInitialized = false;
};