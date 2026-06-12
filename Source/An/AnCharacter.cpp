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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AAnCharacter

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
	CameraBoom->TargetArmLength        = 400.0f;
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
				{
					LanternMID->SetScalarParameterValue(EmissiveParamName, 0.f);
				}
			}
			else
			{
				UE_LOG(LogTemplateCharacter, Warning,
					TEXT("[랜턴] 슬롯 인덱스 %d 초과 — Weapon 슬롯 개수: %d"),
					LanternMaterialIndex, Comp->GetNumMaterials());
			}
			break;
		}
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
		// 점프 — Started 이벤트 때 대화 여부를 판단해 분기
		EIC->BindAction(JumpAction, ETriggerEvent::Started,   this, &AAnCharacter::OnJumpOrAdvance);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// 이동 / 시점
		EIC->BindAction(MoveAction,   ETriggerEvent::Triggered, this, &AAnCharacter::Move);
		EIC->BindAction(LookAction,   ETriggerEvent::Triggered, this, &AAnCharacter::Look);

		// 상호작용
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AAnCharacter::Interact);

		// 대사 넘기기 전용 액션 (에디터에서 DialogueAdvanceAction 에 IA 에셋 할당 필요)
		// JumpAction 과 같은 키를 매핑해도 되고, 별도 IA 를 써도 됩니다.
		if (DialogueAdvanceAction)
		{
			EIC->BindAction(DialogueAdvanceAction, ETriggerEvent::Started, this, &AAnCharacter::AdvanceDialogue);
		}

		// R키 대화 강제 종료 (임시)
		if (DialogueSkipAction)
		{
			EIC->BindAction(DialogueSkipAction, ETriggerEvent::Started, this, &AAnCharacter::EndDialogueInput);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
			TEXT("'%s' Enhanced Input 컴포넌트를 찾지 못했습니다."), *GetNameSafe(this));
	}
}

//////////////////////////////////////////////////////////////////////////
// 대화 상태 진입 / 종료

void AAnCharacter::EnterDialogue(ANpcActor* Npc)
{
	bIsInDialogue = true;
	ActiveNpc     = Npc;

	// 이동 및 점프 비활성화
	GetCharacterMovement()->DisableMovement();
	StopJumping();

	UE_LOG(LogTemplateCharacter, Log, TEXT("[대화] 대화 시작 — 이동·점프 비활성화"));
}

void AAnCharacter::ExitDialogue()
{
	bIsInDialogue = false;
	ActiveNpc     = nullptr;

	// 이동 복원
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	UE_LOG(LogTemplateCharacter, Log, TEXT("[대화] 대화 종료 — 이동·점프 복원"));
}

//////////////////////////////////////////////////////////////////////////
// 스페이스바 분기

void AAnCharacter::OnJumpOrAdvance(const FInputActionValue& Value)
{
	if (bIsInDialogue)
	{
		// 대화 중: 대사 넘기기
		if (ActiveNpc)
		{
			ActiveNpc->AdvanceDialogue(this);
		}
	}
	else
	{
		// 일반 상태: 점프
		Jump();
	}
}

// DialogueAdvanceAction 이 별도로 설정된 경우의 핸들러 (옵션)
void AAnCharacter::AdvanceDialogue(const FInputActionValue& Value)
{
	if (bIsInDialogue && ActiveNpc)
	{
		ActiveNpc->AdvanceDialogue(this);
	}
}

// R키 — 대화 강제 종료 (임시)
void AAnCharacter::EndDialogueInput(const FInputActionValue& Value)
{
	if (bIsInDialogue && ActiveNpc)
	{
		ActiveNpc->EndDialogue(this);
	}
}

//////////////////////////////////////////////////////////////////////////
// 이동 / 시점

void AAnCharacter::Move(const FInputActionValue& Value)
{
	// 대화 중에는 이동 입력 무시 (CharacterMovement 가 Disabled 상태이지만 이중 방어)
	if (bIsInDialogue) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation    = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector  ForwardDir  = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector  RightDir    = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir,   MovementVector.X);
	}
}

void AAnCharacter::Look(const FInputActionValue& Value)
{
	if (bIsInDialogue) return;  // 대화 중 시점 변경 차단 (카메라가 NPC 쪽을 봐야 하므로)

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

//////////////////////////////////////////////////////////////////////////
// 상호작용

void AAnCharacter::Interact(const FInputActionValue& Value)
{
	if (bIsInDialogue) return;  // 대화 중 재상호작용 방지

	AActor* Target = FindInteractableActor();
	if (!Target) return;

	if (Target->Implements<UInteractableInterface>())
	{
		IInteractableInterface::Execute_Interact(Target, this);
	}
}

AActor* AAnCharacter::FindInteractableActor() const
{
	const FVector Center = GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> SweepHits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractDistance);

	bool bSweep = GetWorld()->SweepMultiByChannel(
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

void AAnCharacter::AddLanternEnergy(float Amount)
{
	LanternEnergy = FMath::Max(0.f, LanternEnergy + Amount);

	if (LanternMID)
	{
		const float EmissiveValue = FMath::Clamp(LanternEnergy, 0.f, MaxEmissiveValue);
		LanternMID->SetScalarParameterValue(EmissiveParamName, EmissiveValue);
	}
}