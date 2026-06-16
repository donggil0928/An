// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InteractableInterface.h"
#include "InventoryComponent.h"
#include "NpcActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimInstance.h"   // ABP 프로퍼티 접근
#include "Animation/AnimMontage.h"       // 장착/해제 몽타주

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AAnCharacter::AAnCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	GetCharacterMovement()->bOrientRotationToMovement    = true;
	GetCharacterMovement()->RotationRate                 = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity                = 700.f;
	GetCharacterMovement()->AirControl                   = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed                 = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed           = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking   = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling   = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength         = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	WeaponMesh->SetupAttachment(GetMesh());

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AAnCharacter::BeginPlay()
{
	Super::BeginPlay();

	TArray<USkeletalMeshComponent*> SkelMeshes;
	GetComponents<USkeletalMeshComponent>(SkelMeshes);

	for (USkeletalMeshComponent* Comp : SkelMeshes)
	{
		if (Comp->GetFName() == TEXT("Weapon"))
		{
			if (LanternMaterialIndex < Comp->GetNumMaterials())
			{
				LanternMID = Comp->CreateAndSetMaterialInstanceDynamic(LanternMaterialIndex);
				if (LanternMID)
					LanternMID->SetScalarParameterValue(EmissiveParamName, 0.f);
			}
			break;
		}
	}

	// BP 에디터에서 저장된 값이 C++ 기본값을 덮어쓸 수 있으므로 강제 초기화
	bIsState2 = true;

	// ABP가 준비된 뒤 초기값을 설정하기 위해 초기화 완료 콜백을 등록
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->OnAnimInitialized.AddDynamic(this, &AAnCharacter::OnAnimInitialized);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAnCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &AAnCharacter::OnJumpOrAdvance);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EIC->BindAction(MoveAction,   ETriggerEvent::Triggered, this, &AAnCharacter::Move);
		EIC->BindAction(LookAction,   ETriggerEvent::Triggered, this, &AAnCharacter::Look);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AAnCharacter::Interact);

		if (DialogueAdvanceAction)
			EIC->BindAction(DialogueAdvanceAction, ETriggerEvent::Started, this, &AAnCharacter::AdvanceDialogue);

		if (DialogueSkipAction)
			EIC->BindAction(DialogueSkipAction, ETriggerEvent::Started, this, &AAnCharacter::EndDialogueInput);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
			TEXT("'%s' Enhanced Input 컴포넌트를 찾지 못했습니다."), *GetNameSafe(this));
	}
}

//////////////////////////////////////////////////////////////////////////
// Dialogue

void AAnCharacter::EnterDialogue(ANpcActor* Npc)
{
	bIsInDialogue = true;
	ActiveNpc     = Npc;
	GetCharacterMovement()->DisableMovement();
	StopJumping();
	UE_LOG(LogTemplateCharacter, Log, TEXT("[대화] 시작 — 이동·점프 비활성화"));
}

void AAnCharacter::ExitDialogue()
{
	bIsInDialogue = false;
	ActiveNpc     = nullptr;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[대화] 종료 — 이동·점프 복원"));
}

//////////////////////////////////////////////////////////////////////////
// Lantern

FVector AAnCharacter::GetLanternLocation(FName SocketName) const
{
	if (WeaponMesh)
	{
		if (WeaponMesh->DoesSocketExist(SocketName))
			return WeaponMesh->GetSocketLocation(SocketName);
		return WeaponMesh->GetComponentLocation();
	}
	return GetActorLocation();
}

void AAnCharacter::AddLanternEnergy(float Amount)
{
	LanternEnergy = FMath::Max(0.f, LanternEnergy + Amount);

	if (LanternMID)
	{
		const float EmissiveValue = FMath::Clamp(LanternEnergy, 0.f, MaxEmissiveValue);
		LanternMID->SetScalarParameterValue(EmissiveParamName, EmissiveValue);
	}

	// 장착 완료 상태가 아닐 때만 장착 몽타주 재생
	// bIsLanternEquipped: 장착 몽타주 종료 후 true, 해제 몽타주 종료 후 false
	if (!bIsLanternEquipped)
	{
		UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] 반딧불이 획득 → 장착 몽타주 재생"));
		PlayEquipMontage();
	}
	else
	{
		// 이미 장착(State1) 상태면 타이머만 리셋
		ResetActivityTimers();
	}
}

//////////////////////////////////////////////////////////////////////////
// ABP 초기화 완료 콜백 — 이 시점에는 AnimInstance가 확실히 존재함
void AAnCharacter::OnAnimInitialized()
{
	// 현재 C++ bIsState2 값을 ABP에 동기화만 함
	// 타이머·상태 변경은 하지 않음 (AddLanternEnergy/타이머 콜백이 담당)
	SetABPIsState2(bIsState2);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] ABP 초기화 완료 → bIsState2 = %s 동기화"),
		bIsState2 ? TEXT("true(State2 미장착)") : TEXT("false(State1 장착)"));
	// 콜백 해제: 초기화 이후에는 더 이상 호출될 필요 없음
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->OnAnimInitialized.RemoveDynamic(this, &AAnCharacter::OnAnimInitialized);
	}
}

// [추가] ABP 변수 직접 쓰기

void AAnCharacter::SetABPIsState2(bool bValue)
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[랜턴] ABP 인스턴스 없음 — Is State 2 설정 실패"));
		return;
	}

	// ABP 클래스에서 "bIsState2" 이름의 bool 프로퍼티를 찾아 값을 직접 씁니다.
	// ABP 변수 이름이 다르면 아래 FName을 맞춰 주세요.
	FBoolProperty* Prop = FindFProperty<FBoolProperty>(AnimInst->GetClass(), TEXT("bIsState2"));
	if (Prop)
	{
		Prop->SetPropertyValue_InContainer(AnimInst, bValue);
		UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] ABP bIsState2 = %s"), bValue ? TEXT("true(State2)") : TEXT("false(State1)"));
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning,
			TEXT("[랜턴] ABP에서 'bIsState2' 변수를 찾지 못했습니다. ABP 변수 이름을 확인하세요."));
	}
}

//////////////////////////////////////////////////////////////////////////
// 상태 타이머

void AAnCharacter::ResetActivityTimers()
{
	GetWorldTimerManager().ClearTimer(LanternUnequipTimerHandle);
	GetWorldTimerManager().ClearTimer(IdleTimerHandle);

	// 몽타주 재생 중에는 타이머를 걸지 않음
	// 타이머는 몽타주 종료 콜백(OnEquipMontageEnded)에서 시작됨
	if (bIsPlayingMontage) return;

	if (!bIsState2)
	{
		// 랜턴 장착(State1) 완료 상태 → 5초 후 미장착(State2) 전환
		GetWorldTimerManager().SetTimer(
			LanternUnequipTimerHandle,
			this,
			&AAnCharacter::OnLanternUnequipTimeout,
			InactiveTimeout,
			false
		);
	}
	else
	{
		// 랜턴 미장착(State2) 상태 → 5초 후 Idle 알림
		GetWorldTimerManager().SetTimer(
			IdleTimerHandle,
			this,
			&AAnCharacter::OnIdleTimeout,
			InactiveTimeout,
			false
		);
	}
}

void AAnCharacter::OnLanternUnequipTimeout()
{
	// 장착(State1) → 해제 몽타주 재생 (몽타주 종료 후 State2 전환)
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] 5초 비활성 → 해제 몽타주 재생"));
	PlayUnequipMontage();
}

void AAnCharacter::OnIdleTimeout()
{
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] 5초 추가 비활성 → Idle 상태 진입"));
	// ABP State Machine이 Ground Speed == 0 조건으로 Idle 재생을 담당합니다.
}

//////////////////////////////////////////////////////////////////////////
// 몽타주 재생 및 종료 콜백

void AAnCharacter::PlayEquipMontage()
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst || !EquipMontage)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[랜턴] EquipMontage 없음 — 즉시 State1 전환"));
		bIsState2 = false;
		SetABPIsState2(false);
		ResetActivityTimers();
		return;
	}

	// 해제 몽타주가 재생 중이면 중단
	if (bIsPlayingMontage)
		StopAnimMontage(UnequipMontage);

	bIsPlayingMontage = true;

	// OnMontageEnded 글로벌 델리게이트 등록 (중복 등록 방지)
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnEquipMontageEnded);
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnUnequipMontageEnded);
	AnimInst->OnMontageEnded.AddDynamic(this, &AAnCharacter::OnEquipMontageEnded);

	PlayAnimMontage(EquipMontage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] 장착 몽타주 재생 시작"));
}

void AAnCharacter::PlayUnequipMontage()
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst || !UnequipMontage)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[랜턴] UnequipMontage 없음 — 즉시 State2 전환"));
		bIsState2 = true;
		SetABPIsState2(true);
		GetWorldTimerManager().SetTimer(IdleTimerHandle, this,
			&AAnCharacter::OnIdleTimeout, InactiveTimeout, false);
		return;
	}

	// 장착 몽타주가 재생 중이면 중단
	if (bIsPlayingMontage)
		StopAnimMontage(EquipMontage);

	bIsPlayingMontage = true;

	// OnMontageEnded 글로벌 델리게이트 등록 (중복 등록 방지)
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnEquipMontageEnded);
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnUnequipMontageEnded);
	AnimInst->OnMontageEnded.AddDynamic(this, &AAnCharacter::OnUnequipMontageEnded);

	PlayAnimMontage(UnequipMontage);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] 해제 몽타주 재생 시작"));
}

//////////////////////////////////////////////////////////////////////////
// AnimNotify 콜백

void AAnCharacter::AnimNotify_EquipReady()
{
	// 몽타주 종료 직전 호출 → State1 SM을 미리 활성화해 블렌딩 준비
	// OnEquipMontageEnded보다 먼저 실행되어 SM이 포즈를 준비할 시간을 줌
	bIsState2 = false;
	SetABPIsState2(false);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] EquipReady Notify → State1 미리 전환 (블렌딩 준비)"));
}

void AAnCharacter::AnimNotify_UnequipReady()
{
	// 몽타주 종료 직전 호출 → State2 SM을 미리 활성화해 블렌딩 준비
	bIsState2 = true;
	SetABPIsState2(true);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] UnequipReady Notify → State2 미리 전환 (블렌딩 준비)"));
}

void AAnCharacter::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 다른 몽타주 종료 이벤트는 무시
	if (Montage != EquipMontage) return;

	bIsPlayingMontage = false;

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInst)
		AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnEquipMontageEnded);

	if (bInterrupted)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[랜턴] 장착 몽타주 중단됨"));
		return;
	}

	// Notify가 호출됐으면 이미 State1이지만, 안 됐을 경우를 대비해 여기서도 보장
	bIsState2 = false;
	SetABPIsState2(false);
	bIsLanternEquipped = true;   // 장착 완료
	ResetActivityTimers();
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] 장착 몽타주 완료 → State1(장착) 전환"));
}

void AAnCharacter::OnUnequipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 다른 몽타주 종료 이벤트는 무시
	if (Montage != UnequipMontage) return;

	bIsPlayingMontage = false;

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInst)
		AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnUnequipMontageEnded);

	if (bInterrupted)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("[랜턴] 해제 몽타주 중단됨"));
		return;
	}

	// Notify가 호출됐으면 이미 State2이지만, 안 됐을 경우를 대비해 여기서도 보장
	bIsState2 = true;
	SetABPIsState2(true);
	bIsLanternEquipped = false;  // 장착 해제 완료
	GetWorldTimerManager().SetTimer(IdleTimerHandle, this,
		&AAnCharacter::OnIdleTimeout, InactiveTimeout, false);
	UE_LOG(LogTemplateCharacter, Log, TEXT("[랜턴] 해제 몽타주 완료 → State2(미장착) 전환"));
}

//////////////////////////////////////////////////////////////////////////
// Move / Look / Interact

void AAnCharacter::OnJumpOrAdvance(const FInputActionValue& Value)
{
	if (bIsInDialogue)
	{
		if (ActiveNpc) ActiveNpc->AdvanceDialogue(this);
	}
	else
	{
		Jump();
	}
}

void AAnCharacter::AdvanceDialogue(const FInputActionValue& Value)
{
	if (bIsInDialogue && ActiveNpc)
		ActiveNpc->AdvanceDialogue(this);
}

void AAnCharacter::EndDialogueInput(const FInputActionValue& Value)
{
	if (bIsInDialogue && ActiveNpc)
		ActiveNpc->EndDialogue(this);
}

void AAnCharacter::Move(const FInputActionValue& Value)
{
	if (bIsInDialogue) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation   = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector  ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector  RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir,   MovementVector.X);
	}

	// 이동 입력 → 장착(State1) 상태일 때만 타이머 리셋
	if (!bIsState2)
		ResetActivityTimers();
}

void AAnCharacter::Look(const FInputActionValue& Value)
{
	if (bIsInDialogue) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAnCharacter::Interact(const FInputActionValue& Value)
{
	if (bIsInDialogue) return;

	AActor* Target = FindInteractableActor();
	if (!Target) return;

	if (Target->Implements<UInteractableInterface>())
		IInteractableInterface::Execute_Interact(Target, this);
}

AActor* AAnCharacter::FindInteractableActor() const
{
	const FVector Center = GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> SweepHits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractDistance);

	GetWorld()->SweepMultiByChannel(
		SweepHits,
		Center,
		Center + FVector(0.f, 0.f, 1.f),
		FQuat::Identity,
		ECC_Visibility,
		Sphere,
		Params
	);

	AActor* ClosestActor = nullptr;
	float   ClosestDist  = FLT_MAX;

	for (auto& H : SweepHits)
	{
		AActor* A = H.GetActor();
		if (!A || !A->Implements<UInteractableInterface>()) continue;

		float Dist = FVector::Dist(Center, A->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist  = Dist;
			ClosestActor = A;
		}
	}

	return ClosestActor;
}