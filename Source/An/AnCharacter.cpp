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
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h" 

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

void AAnCharacter::AttachWeaponToSocket(FName SocketName)
{
	if (!WeaponMesh || !GetMesh()) return;

	FAttachmentTransformRules Rules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepRelative,
		true 
	);
	WeaponMesh->AttachToComponent(GetMesh(), Rules, SocketName);
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

	bIsState2 = true;
	SetABPIsState2(bIsState2);
	bIsLanternEquipped = false;
	AttachWeaponToSocket(LanternDefaultSocket);
}

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
}

void AAnCharacter::EnterDialogue(ANpcActor* Npc)
{
	bIsInDialogue = true;
	ActiveNpc     = Npc;
	GetCharacterMovement()->DisableMovement();
	StopJumping();
}

void AAnCharacter::ExitDialogue()
{
	bIsInDialogue = false;
	ActiveNpc     = nullptr;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

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
	
	if (!bIsLanternEquipped && !bIsPlayingMontage)
	{
		PlayEquipMontage();
	}
	else
	{
		ResetActivityTimers();
	}
}

void AAnCharacter::SetABPIsState2(bool bValue)
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;
	
	FBoolProperty* Prop = FindFProperty<FBoolProperty>(AnimInst->GetClass(), TEXT("bIsState2"));
	if (Prop)
	{
		Prop->SetPropertyValue_InContainer(AnimInst, bValue);
	}
}

void AAnCharacter::ResetActivityTimers()
{
	GetWorldTimerManager().ClearTimer(LanternUnequipTimerHandle);
	GetWorldTimerManager().ClearTimer(IdleTimerHandle);
	
	if (bIsPlayingMontage) return;

	if (!bIsState2)
	{
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
	PlayUnequipMontage();
}

void AAnCharacter::OnIdleTimeout()
{
}

void AAnCharacter::PlayEquipMontage()
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst || !EquipMontage)
	{
		bIsState2 = false;
		SetABPIsState2(false);
		ResetActivityTimers();
		return;
	}

	bIsState2 = false;
	SetABPIsState2(false);
	
	if (bIsPlayingMontage)
		StopAnimMontage(UnequipMontage);

	bIsPlayingMontage = true;
	
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnEquipMontageEnded);
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnUnequipMontageEnded);
	AnimInst->OnMontageEnded.AddDynamic(this, &AAnCharacter::OnEquipMontageEnded);

	PlayAnimMontage(EquipMontage);
}

void AAnCharacter::PlayUnequipMontage()
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst || !UnequipMontage)
	{
		bIsState2 = true;
		SetABPIsState2(true);
		GetWorldTimerManager().SetTimer(IdleTimerHandle, this,
			&AAnCharacter::OnIdleTimeout, InactiveTimeout, false);
		return;
	}

	bIsState2 = true;
	SetABPIsState2(true);
	
	if (bIsPlayingMontage)
		StopAnimMontage(EquipMontage);

	bIsPlayingMontage = true;
	
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnEquipMontageEnded);
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnUnequipMontageEnded);
	AnimInst->OnMontageEnded.AddDynamic(this, &AAnCharacter::OnUnequipMontageEnded);

	PlayAnimMontage(UnequipMontage);
}

void AAnCharacter::AnimNotify_EquipReady()
{
	bIsState2 = false;
	SetABPIsState2(false);
	bIsLanternEquipped = true;
	AttachWeaponToSocket(LanternEquipSocket);
}

void AAnCharacter::AnimNotify_UnequipReady()
{
	bIsState2 = true;
	SetABPIsState2(true);
	bIsLanternEquipped = false;
	AttachWeaponToSocket(LanternDefaultSocket);
}

void AAnCharacter::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != EquipMontage) return;

	bIsPlayingMontage = false;

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInst)
		AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnEquipMontageEnded);

	if (bInterrupted) return;

	bIsState2 = false;
	SetABPIsState2(false);
	bIsLanternEquipped = true;
	ResetActivityTimers();

	OnLanternEquippedComplete.Broadcast();
}

void AAnCharacter::OnUnequipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != UnequipMontage) return;

	bIsPlayingMontage = false;

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInst)
		AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnUnequipMontageEnded);

	if (bInterrupted)
	{
		return;
	}
	
	bIsState2 = true;
	SetABPIsState2(true);
	bIsLanternEquipped = false;
	AttachWeaponToSocket(LanternDefaultSocket);

	GetWorldTimerManager().SetTimer(IdleTimerHandle, this,
		&AAnCharacter::OnIdleTimeout, InactiveTimeout, false);
}

void AAnCharacter::PlayCollectMontage()
{
	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst || !CollectMontage)
	{
		bIsCollecting = false;
		return;
	}

	bIsCollecting = true;

	AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnCollectMontageEnded);
	AnimInst->OnMontageEnded.AddDynamic(this, &AAnCharacter::OnCollectMontageEnded);

	PlayAnimMontage(CollectMontage);
}

void AAnCharacter::OnCollectMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CollectMontage) return;

	bIsCollecting = false;

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInst)
		AnimInst->OnMontageEnded.RemoveDynamic(this, &AAnCharacter::OnCollectMontageEnded);
}

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
	if (bIsCollecting) return;

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