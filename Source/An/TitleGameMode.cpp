#include "TitleGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATitleGameMode::ATitleGameMode()
{
	DefaultPawnClass = nullptr;
}

void ATitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	PC->bShowMouseCursor    = true;
	PC->SetInputMode(FInputModeUIOnly());

	if (TitleWidgetClass)
	{
		TitleWidget = CreateWidget<UUserWidget>(PC, TitleWidgetClass);
		if (TitleWidget)
		{
			if (FObjectProperty* Prop = FindFProperty<FObjectProperty>(
				TitleWidget->GetClass(), TEXT("OwnerGameMode")))
			{
				Prop->SetObjectPropertyValue_InContainer(TitleWidget, this);
			}

			TitleWidget->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TitleGameMode] TitleWidgetClass 미설정 — BP_TitleGameMode 에서 할당하세요."));
	}
}

void ATitleGameMode::StartGame()
{
	if (LoadingWidget) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (TitleWidget)
	{
		TitleWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (LoadingWidgetClass)
	{
		LoadingWidget = CreateWidget<UUserWidget>(PC, LoadingWidgetClass);
		if (LoadingWidget)
		{
			LoadingWidget->AddToViewport(10);
		}
	}

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		this,
		&ATitleGameMode::OpenGameLevel,
		LoadingMinTime,
		false
	);
}

void ATitleGameMode::OpenGameLevel()
{

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	UGameplayStatics::OpenLevel(GetWorld(), GameLevelName);
}