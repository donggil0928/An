#include "TitleWidget.h"
#include "TitleGameMode.h"
#include "Components/Button.h"

void UTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_StartGame)
	{
		Btn_StartGame->OnClicked.AddDynamic(this, &UTitleWidget::OnStartGameClicked);
	}
	
	auto DisableBtn = [](UButton* Btn)
	{
		if (!Btn) return;
		Btn->SetIsEnabled(false);
		Btn->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.35f));
	};

	DisableBtn(Btn_Continue);
	DisableBtn(Btn_Setting);
	DisableBtn(Btn_Exit);
}

void UTitleWidget::OnStartGameClicked()
{
	if (OwnerGameMode)
	{
		OwnerGameMode->StartGame();
	}
	else
	{
		if (ATitleGameMode* GM = Cast<ATitleGameMode>(
			GetWorld()->GetAuthGameMode()))
		{
			GM->StartGame();
		}
	}
}
