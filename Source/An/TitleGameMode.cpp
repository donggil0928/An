#include "TitleGameMode.h"
#include "LoadingWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/LevelStreamingDynamic.h"

ATitleGameMode::ATitleGameMode()
{
	DefaultPawnClass = nullptr;
	PrimaryActorTick.bCanEverTick         = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ATitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	PC->bShowMouseCursor = true;
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
		UE_LOG(LogTemp, Error, TEXT("[TitleGameMode] TitleWidgetClass 미설정"));
	}
}

void ATitleGameMode::StartGame()
{
	if (bIsLoading) return;
	if (!GameLevel.IsValid() && GameLevel.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[TitleGameMode] GameLevel 미설정 — BP_TitleGameMode 에서 레벨 에셋을 할당하세요."));
		return;
	}

	bIsLoading      = true;
	bLevelLoaded    = false;
	bMinTimeElapsed = false;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;
	
	if (TitleWidget)
		TitleWidget->SetVisibility(ESlateVisibility::Hidden);
	
	if (LoadingWidgetClass)
	{
		LoadingWidget = CreateWidget<ULoadingWidget>(PC, LoadingWidgetClass);
		if (LoadingWidget)
			LoadingWidget->AddToViewport(10);
	}
	
	bool bSuccess = false;
	StreamingLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		GetWorld(),
		GameLevel,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		bSuccess
	);

	if (!bSuccess || !StreamingLevel)
	{
		UE_LOG(LogTemp, Error, TEXT("[TitleGameMode] 레벨 스트리밍 시작 실패"));
		bIsLoading = false;
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[TitleGameMode] 비동기 스트리밍 시작: %s"), *GameLevel.ToString());
	
	GetWorldTimerManager().SetTimer(
		MinTimeHandle,
		this,
		&ATitleGameMode::OnMinTimeElapsed,
		LoadingMinTime,
		false
	);
	
	SetActorTickEnabled(true);
}

void ATitleGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsLoading || !StreamingLevel) return;
	
	const ELevelStreamingState State = StreamingLevel->GetLevelStreamingState();

	float Progress = 0.f;
	switch (State)
	{
	case ELevelStreamingState::Loading:
		Progress = 0.5f;
		break;
	case ELevelStreamingState::LoadedNotVisible:
	case ELevelStreamingState::LoadedVisible:
		Progress = 1.f;
		break;
	default:
		Progress = 0.f;
		break;
	}

	if (LoadingWidget)
		LoadingWidget->SetLoadingProgress(Progress);
	
	if (!bLevelLoaded &&
		(State == ELevelStreamingState::LoadedNotVisible ||
		 State == ELevelStreamingState::LoadedVisible))
	{
		UE_LOG(LogTemp, Log, TEXT("[TitleGameMode] 레벨 스트리밍 완료"));
		bLevelLoaded = true;
		TryOpenGameLevel();
	}
}

void ATitleGameMode::OnMinTimeElapsed()
{
	UE_LOG(LogTemp, Log, TEXT("[TitleGameMode] 최소 로딩 시간 경과"));
	bMinTimeElapsed = true;
	TryOpenGameLevel();
}

void ATitleGameMode::TryOpenGameLevel()
{
	if (!bLevelLoaded || !bMinTimeElapsed) return;

	SetActorTickEnabled(false);

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
	
	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(GameLevel.ToString()));
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}