#include "EndWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UEndWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_ToTitle)
	{
		Btn_ToTitle->OnClicked.AddDynamic(this, &UEndWidget::OnToTitleClicked);
	}
}

void UEndWidget::OnToTitleClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TitleLevelName);
}