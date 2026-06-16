#include "LoadingWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/ProgressBar.h"

void ULoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ProgressBar_Loading)
		ProgressBar_Loading->SetPercent(0.f);
	
	if (PlayAnim)
		PlayAnimation(PlayAnim, 0.f, 0 /* 0 = 무한 루프 */);
}

void ULoadingWidget::SetLoadingProgress(float InProgress)
{
	if (ProgressBar_Loading)
		ProgressBar_Loading->SetPercent(FMath::Clamp(InProgress, 0.f, 1.f));
}