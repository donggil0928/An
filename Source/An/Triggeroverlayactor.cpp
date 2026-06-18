#include "TriggerOverlayActor.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATriggerOverlayActor::ATriggerOverlayActor()
{
	PrimaryActorTick.bCanEverTick  = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ATriggerOverlayActor::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(
		this, &ATriggerOverlayActor::OnBoxBeginOverlap);
}

void ATriggerOverlayActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ActiveWidget) return;

	FadeElapsed += DeltaSeconds;

	switch (FadeState)
	{
	case EFadeState::FadeIn:
	{
		const float Alpha = FMath::Clamp(FadeElapsed / FadeInDuration, 0.f, 1.f);
		ActiveWidget->SetRenderOpacity(Alpha);

		if (FadeElapsed >= FadeInDuration)
			StartHold();

		break;
	}
	case EFadeState::FadeOut:
	{
		const float Alpha = FMath::Clamp(1.f - (FadeElapsed / FadeOutDuration), 0.f, 1.f);
		ActiveWidget->SetRenderOpacity(Alpha);

		if (FadeElapsed >= FadeOutDuration)
			FinishOverlay();

		break;
	}
	default:
		break;
	}
}

void ATriggerOverlayActor::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (OtherActor != PlayerChar) return;

	if (bTriggerOnce && bHasTriggered) return;
	bHasTriggered = true;

	StartFadeIn();
}

void ATriggerOverlayActor::StartFadeIn()
{
	if (!OverlayWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TriggerOverlay] OverlayWidgetClass 미설정"));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;
	
	GetWorldTimerManager().ClearTimer(HoldTimerHandle);
	GetWorldTimerManager().ClearTimer(FadeOutTimerHandle);

	if (!ActiveWidget)
	{
		ActiveWidget = CreateWidget<UUserWidget>(PC, OverlayWidgetClass);
		if (!ActiveWidget) return;
		ActiveWidget->AddToViewport(WidgetZOrder);
	}

	ActiveWidget->SetRenderOpacity(0.f);
	FadeState   = EFadeState::FadeIn;
	FadeElapsed = 0.f;

	SetActorTickEnabled(true);

	UE_LOG(LogTemp, Log, TEXT("[TriggerOverlay] 페이드인 시작 (%.1f초)"), FadeInDuration);
}

void ATriggerOverlayActor::StartHold()
{
	if (ActiveWidget)
		ActiveWidget->SetRenderOpacity(1.f);

	FadeState   = EFadeState::Hold;
	FadeElapsed = 0.f;
	
	SetActorTickEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("[TriggerOverlay] 유지 시작 (%.1f초)"), DisplayDuration);

	GetWorldTimerManager().SetTimer(
		HoldTimerHandle,
		this,
		&ATriggerOverlayActor::StartFadeOut,
		DisplayDuration,
		false
	);
}

void ATriggerOverlayActor::StartFadeOut()
{
	FadeState   = EFadeState::FadeOut;
	FadeElapsed = 0.f;

	SetActorTickEnabled(true);

	UE_LOG(LogTemp, Log, TEXT("[TriggerOverlay] 페이드아웃 시작 (%.1f초)"), FadeOutDuration);
}

void ATriggerOverlayActor::FinishOverlay()
{
	SetActorTickEnabled(false);
	FadeState = EFadeState::Idle;

	if (ActiveWidget)
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("[TriggerOverlay] 오버레이 종료"));
}