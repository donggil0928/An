#include "PortalActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "AnCharacter.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimInstance.h"	
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

APortalActor::APortalActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
}

void APortalActor::Interact_Implementation(AActor* Interactor)
{
	if (bIsActivated) return;

	AAnCharacter* Player = Cast<AAnCharacter>(Interactor);
	if (!Player) return;

	const float CurrentEnergy = Player->GetLanternEnergy();

	if (CurrentEnergy < RequiredEnergy)
	{
		UE_LOG(LogTemp, Log, TEXT("[포탈] 에너지 부족 (%.1f / %.1f)"), CurrentEnergy, RequiredEnergy);
		OnPortalLocked(CurrentEnergy, RequiredEnergy);
		return;
	}

	bIsActivated      = true;
	CurrentInteractor = Interactor;

	UE_LOG(LogTemp, Log, TEXT("[포탈] 활성화 — 포탈이 열립니다."));
	
	OnPortalOpen();

	PlayOpenMontage();
	
	PlayOpenCinematic();
	
	StartEndWidgetTimer();
}

void APortalActor::PlayOpenCinematic()
{
	if (!OpenSequence)
	{
		UE_LOG(LogTemp, Warning, TEXT("[포탈] OpenSequence 미설정 — 시네마틱 생략"));
		return;
	}
	
	if (AAnCharacter* Player = Cast<AAnCharacter>(CurrentInteractor))
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			Player->DisableInput(PC);
		}
	}

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bDisableMovementInput     = true;
	Settings.bDisableLookAtInput       = true;
	Settings.bHidePlayer               = false;

	ALevelSequenceActor* SeqActor = nullptr;
	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(), OpenSequence, Settings, SeqActor);

	if (SequencePlayer)
	{
		SequencePlayer->OnFinished.AddDynamic(this, &APortalActor::OnSequenceFinished);
		SequencePlayer->Play();
	}
}

void APortalActor::PlayOpenMontage()
{
	if (!OpenMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[포탈] OpenMontage 미설정 — 몽타주 생략"));
		return;
	}

	UAnimInstance* AnimInst = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (!AnimInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("[포탈] AnimInstance 없음 — 메시에 Anim Class 미지정"));
		return;
	}

	AnimInst->Montage_Play(OpenMontage);
}

void APortalActor::StartEndWidgetTimer()
{
	float TotalDelay = EndWidgetDelay;

	if (OpenSequence && SequencePlayer)
	{
		TotalDelay += SequencePlayer->GetDuration().AsSeconds();
	}

	UE_LOG(LogTemp, Log, TEXT("[포탈] %.1f초 후 END 위젯 표시"), TotalDelay);

	GetWorldTimerManager().SetTimer(
		EndWidgetTimerHandle,
		this,
		&APortalActor::ShowEndWidget,
		TotalDelay,
		false
	);
}

void APortalActor::ShowEndWidget()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (!EndWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[포탈] EndWidgetClass 미설정"));
		return;
	}

	if (UUserWidget* EndWidget = CreateWidget<UUserWidget>(PC, EndWidgetClass))
	{
		EndWidget->AddToViewport(100);
	}
	
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeUIOnly());
}

void APortalActor::OnSequenceFinished()
{
	UE_LOG(LogTemp, Log, TEXT("[포탈] 시네마틱 종료"));
}

EInteractableType APortalActor::GetInteractableType_Implementation() const
{
	return EInteractableType::Portal;
}