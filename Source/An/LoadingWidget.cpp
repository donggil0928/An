#include "LoadingWidget.h"
#include "Animation/WidgetAnimation.h"

void ULoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();


	if (PlayAnim)
	{
		PlayAnimation(PlayAnim, 0.f, 0 /* 0 = 무한 루프 */);
	}
}
