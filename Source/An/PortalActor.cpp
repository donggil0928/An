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
	UE_LOG(LogTemp, Warning, TEXT("[포탈] Interact 호출됨"));
    
    	if (bIsActivated)
    	{
    		UE_LOG(LogTemp, Warning, TEXT("[포탈] 이미 활성화됨 — 중복 호출 무시"));
    		return;
    	}
    
    	AAnCharacter* Player = Cast<AAnCharacter>(Interactor);
    	if (!Player)
    	{
    		UE_LOG(LogTemp, Error, TEXT("[포탈] Interactor가 AAnCharacter 아님 — 캐스트 실패"));
    		return;
    	}
    
    	const float CurrentEnergy = Player->GetLanternEnergy();
    	UE_LOG(LogTemp, Warning, TEXT("[포탈] 현재 에너지 %.1f / 필요 %.1f"), CurrentEnergy, RequiredEnergy);
    
    	// ★ 테스트 플래그가 꺼져 있을 때만 에너지 조건 적용
    	if (!bIgnoreEnergyForTest && CurrentEnergy < RequiredEnergy)
    	{
    		UE_LOG(LogTemp, Log, TEXT("[포탈] 에너지 부족 — 활성화 취소"));
    		OnPortalLocked(CurrentEnergy, RequiredEnergy);
    		return;
    	}
    
    	bIsActivated      = true;
    	CurrentInteractor = Interactor;
    
    	UE_LOG(LogTemp, Warning, TEXT("[포탈] 활성화 성공 — 연출 시작"));
    
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
    		const float SeqLen = SequencePlayer->GetDuration().AsSeconds();
    		TotalDelay += SeqLen;
    		UE_LOG(LogTemp, Warning, TEXT("[포탈] 시퀀스 있음 — 길이 %.1f + 지연 %.1f = %.1f초 후 표시"),
    			SeqLen, EndWidgetDelay, TotalDelay);
    	}
    	else
    	{
    		UE_LOG(LogTemp, Warning, TEXT("[포탈] 시퀀스 없음 — %.1f초 후 표시 (OpenSequence=%s, SequencePlayer=%s)"),
    			TotalDelay,
    			OpenSequence  ? TEXT("OK") : TEXT("NULL"),
    			SequencePlayer ? TEXT("OK") : TEXT("NULL"));
    	}
    
    	GetWorldTimerManager().SetTimer(
    		EndWidgetTimerHandle,
    		this,
    		&APortalActor::ShowEndWidget,
    		TotalDelay,
    		false
    	);
    
    	UE_LOG(LogTemp, Warning, TEXT("[포탈] END 위젯 타이머 설정 완료 (%.1f초)"), TotalDelay);
}

void APortalActor::ShowEndWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("[포탈] ShowEndWidget 호출됨 — 타이머 만료"));

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[포탈] PlayerController 없음"));
		return;
	}

	if (!EndWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[포탈] EndWidgetClass 미설정 — 위젯 생성 불가"));
		return;
	}

	if (UUserWidget* EndWidget = CreateWidget<UUserWidget>(PC, EndWidgetClass))
	{
		EndWidget->AddToViewport(100);
		UE_LOG(LogTemp, Warning, TEXT("[포탈] END 위젯 생성 및 뷰포트 추가 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[포탈] CreateWidget 실패"));
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