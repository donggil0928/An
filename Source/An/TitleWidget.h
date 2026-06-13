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

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_StartGame;
	
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Continue;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Setting;

	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* Btn_Exit;

	UFUNCTION()
	void OnStartGameClicked();
};
