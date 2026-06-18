#include "PauseWidget.h"
#include "AnCharacter.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Btn_Continue)
		Btn_Continue->OnClicked.AddDynamic(this, &UPauseWidget::OnContinueClicked);

	if (Btn_MainMenu)
		Btn_MainMenu->OnClicked.AddDynamic(this, &UPauseWidget::OnMainMenuClicked);

	if (Btn_Exit)
		Btn_Exit->OnClicked.AddDynamic(this, &UPauseWidget::OnExitClicked);
	
	BindButtonHover(Btn_Continue,  TEXT("OnHoverContinue"));
	BindButtonHover(Btn_Settings,  TEXT("OnHoverSettings"));
	BindButtonHover(Btn_MainMenu,  TEXT("OnHoverMainMenu"));
	BindButtonHover(Btn_Exit,      TEXT("OnHoverExit"));

	CurrentTargetBtn  = Btn_Continue;
	bArrowInitialized = false;
}

void UPauseWidget::OnContinueClicked()
{
	if (OwnerCharacter)
		OwnerCharacter->TogglePause();
}

void UPauseWidget::OnMainMenuClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetPause(false);
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("TitleMap")); // 블루프린트 BP에서 레벨명 일치 확인
}

void UPauseWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(),
		EQuitPreference::Quit, false);
}

void UPauseWidget::OnHoverContinue() { SetArrowTargetButton(Btn_Continue); }
void UPauseWidget::OnHoverSettings() { SetArrowTargetButton(Btn_Settings);  }
void UPauseWidget::OnHoverMainMenu() { SetArrowTargetButton(Btn_MainMenu);  }
void UPauseWidget::OnHoverExit()     { SetArrowTargetButton(Btn_Exit);      }
void UPauseWidget::OnUnhover()       {  }


void UPauseWidget::SetArrowTargetButton(UButton* Btn)
{
	if (Btn) CurrentTargetBtn = Btn;
}

void UPauseWidget::BindButtonHover(UButton* Btn, FName HoverFuncName)
{
	if (!Btn) return;

	FScriptDelegate HoverDel;
	HoverDel.BindUFunction(this, HoverFuncName);
	Btn->OnHovered.Add(HoverDel);

	FScriptDelegate UnhoverDel;
	UnhoverDel.BindUFunction(this, TEXT("OnUnhover"));
	Btn->OnUnhovered.Add(UnhoverDel);
}


void UPauseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Img_Arrow || !CurrentTargetBtn) return;

	UCanvasPanelSlot* ArrowSlot = Cast<UCanvasPanelSlot>(Img_Arrow->Slot);
	if (!ArrowSlot) return;

	const FGeometry& BtnGeo      = CurrentTargetBtn->GetCachedGeometry();
	const FVector2D  BtnAbsSize  = BtnGeo.GetAbsoluteSize();
	if (BtnAbsSize.IsZero()) return;

	const FVector2D BtnLeftCenterLocal(0.f, BtnGeo.GetLocalSize().Y * 0.5f);
	const FVector2D BtnLeftCenterAbs     = BtnGeo.LocalToAbsolute(BtnLeftCenterLocal);
	const FVector2D BtnLeftCenterCanvas  = MyGeometry.AbsoluteToLocal(BtnLeftCenterAbs);

	const FVector2D ArrowSize = ArrowSlot->GetSize();

	FVector2D TargetPos;
	TargetPos.X = BtnLeftCenterCanvas.X - ArrowSize.X - ArrowGapX;
	TargetPos.Y = BtnLeftCenterCanvas.Y - ArrowSize.Y * 0.5f;

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