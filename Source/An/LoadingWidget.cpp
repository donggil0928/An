#include "LoadingWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"

void ULoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TargetProgress = 0.f;
	CurrentProgress = 0.f;
	
	if (Image_Lamp)
	{
		LampMaterialInstance = Image_Lamp->GetDynamicMaterial();
		if (LampMaterialInstance)
		{
			LampMaterialInstance->SetScalarParameterValue(FName("FillAmount"), 0.f);
		}
	}
	
	if (PlayAnim)
		PlayAnimation(PlayAnim, 0.f, 0);
}

void ULoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (LampMaterialInstance && CurrentProgress < TargetProgress)
	{
		CurrentProgress = FMath::FInterpTo(CurrentProgress, TargetProgress, InDeltaTime, 5.f);
		LampMaterialInstance->SetScalarParameterValue(FName("FillAmount"), CurrentProgress);
	}
}

void ULoadingWidget::SetLoadingProgress(float InProgress)
{
	if (InProgress > TargetProgress)
	{
		TargetProgress = FMath::Clamp(InProgress, 0.f, 1.f);
	}
}
