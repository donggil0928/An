#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseWidget.generated.h"

UCLASS()
class UPauseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Pause")
	class AAnCharacter* OwnerCharacter;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Continue;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Settings;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_MainMenu;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Exit;

	UPROPERTY(meta = (BindWidget))
	class UImage* Img_Arrow;

	UFUNCTION()
	void OnContinueClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnExitClicked();
	
	UFUNCTION() void OnHoverContinue();
	UFUNCTION() void OnHoverSettings();
	UFUNCTION() void OnHoverMainMenu();
	UFUNCTION() void OnHoverExit();
	UFUNCTION() void OnUnhover();
	
	void SetArrowTargetButton(class UButton* Btn);
	void BindButtonHover(class UButton* Btn, FName HoverFuncName);

	UPROPERTY()
	class UButton* CurrentTargetBtn = nullptr;

	float ArrowInterpSpeed = 12.f;
	float ArrowGapX        = 20.f;
	bool  bArrowInitialized = false;
};