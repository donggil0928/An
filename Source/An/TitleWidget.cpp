#include "TitleWidget.h"
#include "TitleGameMode.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

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
	
	BindButtonHover(Btn_StartGame, TEXT("OnHoverStartGame"));
	BindButtonHover(Btn_Continue,  TEXT("OnHoverContinue"));
	BindButtonHover(Btn_Setting,   TEXT("OnHoverSetting"));
	BindButtonHover(Btn_Exit,      TEXT("OnHoverExit"));
	
	DefaultBtn       = Btn_StartGame;
	CurrentTargetBtn = Btn_StartGame;

	bArrowInitialized = false;
}

void UTitleWidget::BindButtonHover(UButton* Btn, FName HoverFuncName)
{
	if (!Btn) return;

	FScriptDelegate HoverDel;
	HoverDel.BindUFunction(this, HoverFuncName);
	Btn->OnHovered.Add(HoverDel);

	FScriptDelegate UnhoverDel;
	UnhoverDel.BindUFunction(this, TEXT("OnUnhover"));
	Btn->OnUnhovered.Add(UnhoverDel);
}

void UTitleWidget::OnHoverStartGame() { SetArrowTargetButton(Btn_StartGame); }
void UTitleWidget::OnHoverContinue()  { SetArrowTargetButton(Btn_Continue); }
void UTitleWidget::OnHoverSetting()   { SetArrowTargetButton(Btn_Setting); }
void UTitleWidget::OnHoverExit()      { SetArrowTargetButton(Btn_Exit); }

void UTitleWidget::OnUnhover()
{
	//SetArrowTargetButton(DefaultBtn);
}

void UTitleWidget::SetArrowTargetButton(UButton* Btn)
{
	if (Btn)
		CurrentTargetBtn = Btn;
}

void UTitleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Img_Arrow || !CurrentTargetBtn) return;

	UCanvasPanelSlot* ArrowSlot = Cast<UCanvasPanelSlot>(Img_Arrow->Slot);
	if (!ArrowSlot) return;
	
	const FGeometry& BtnGeo = CurrentTargetBtn->GetCachedGeometry();
	const FVector2D BtnAbsSize = BtnGeo.GetAbsoluteSize();
	if (BtnAbsSize.IsZero()) return;
	
	const FVector2D BtnLeftCenterLocal(0.f, BtnGeo.GetLocalSize().Y * 0.5f);
	const FVector2D BtnLeftCenterAbs = BtnGeo.LocalToAbsolute(BtnLeftCenterLocal);
	
	const FVector2D BtnLeftCenterInCanvas = MyGeometry.AbsoluteToLocal(BtnLeftCenterAbs);
	
	const FVector2D ArrowSize = ArrowSlot->GetSize();

	FVector2D TargetPos;
	TargetPos.X = BtnLeftCenterInCanvas.X - ArrowSize.X - ArrowGapX;
	TargetPos.Y = BtnLeftCenterInCanvas.Y - ArrowSize.Y * 0.5f;

	if (!bArrowInitialized)
	{
		ArrowSlot->SetPosition(TargetPos);
		bArrowInitialized = true;
		return;
	}

	const FVector2D CurPos = ArrowSlot->GetPosition();
	const FVector2D NewPos = FMath::Vector2DInterpTo(CurPos, TargetPos, InDeltaTime, ArrowInterpSpeed);
	ArrowSlot->SetPosition(NewPos);
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