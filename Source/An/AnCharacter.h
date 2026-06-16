// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AnCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UInventoryComponent;
class ANpcActor;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AAnCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DialogueAdvanceAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DialogueSkipAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float InteractDistance = 200.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lantern", meta = (AllowPrivateAccess = "true"))
	float LanternEnergy = 0.f;

	UPROPERTY(EditAnywhere, Category = "Lantern", meta = (AllowPrivateAccess = "true"))
	float MaxEmissiveValue = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lantern", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, Category = "Lantern", meta = (AllowPrivateAccess = "true"))
	int32 LanternMaterialIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Lantern", meta = (AllowPrivateAccess = "true"))
	FName EmissiveParamName = TEXT("EmissiveIntensity");

	UPROPERTY()
	class UMaterialInstanceDynamic* LanternMID;

	UPROPERTY()
	ANpcActor* ActiveNpc;

	bool bIsInDialogue = false;

	// ────────────────────────────────────────────────
	// 랜턴 상태
	// State1 = 랜턴 장착(bIsState2 = false)
	// State2 = 랜턴 비장착(bIsState2 = true)
	// ────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lantern|State", meta = (AllowPrivateAccess = "true"))
	bool bIsState2 = true;

	UPROPERTY(EditAnywhere, Category = "Lantern|State", meta = (AllowPrivateAccess = "true"))
	float InactiveTimeout = 5.f;

	// [추가] 장착/해제 몽타주 애셋 — BP에서 AM_LanternEquip / AM_LanternUnequip 할당
	UPROPERTY(EditAnywhere, Category = "Lantern|Montage", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere, Category = "Lantern|Montage", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* UnequipMontage;

	// [추가] 몽타주 재생 중 상태 전환 차단 플래그
	bool bIsPlayingMontage = false;

	// [추가] 랜턴 장착 완료 여부 (몽타주 종료 후 State1 진입 시 true)
	// bIsState2와 별개로, 몽타주 재생 중에도 "장착 의도" 여부를 추적
	bool bIsLanternEquipped = false;

	FTimerHandle LanternUnequipTimerHandle;
	FTimerHandle IdleTimerHandle;

	void ResetActivityTimers();
	void OnLanternUnequipTimeout();
	void OnIdleTimeout();

	void SetABPIsState2(bool bValue);

	// [추가] 몽타주 재생 및 종료 콜백
	void PlayEquipMontage();
	void PlayUnequipMontage();

	UFUNCTION()
	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnUnequipMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnAnimInitialized();

	// [추가] 몽타주 AnimNotify 콜백
	// AM_LanternEquip 타임라인에 "NotifyEquipReady" 추가 후 연결
	UFUNCTION(BlueprintCallable, Category = "Lantern|Montage")
	void AnimNotify_EquipReady();

	// AM_LanternUnequip 타임라인에 "NotifyUnequipReady" 추가 후 연결
	UFUNCTION(BlueprintCallable, Category = "Lantern|Montage")
	void AnimNotify_UnequipReady();

public:
	AAnCharacter();

	void EnterDialogue(ANpcActor* Npc);
	void ExitDialogue();

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	FVector GetLanternLocation(FName SocketName = TEXT("lantern_socket")) const;

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	void AddLanternEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	float GetLanternEnergy() const { return LanternEnergy; }

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }

	UFUNCTION(BlueprintCallable, Category = "Lantern|State")
	bool GetIsState2() const { return bIsState2; }

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void OnJumpOrAdvance(const FInputActionValue& Value);
	void AdvanceDialogue(const FInputActionValue& Value);
	void EndDialogueInput(const FInputActionValue& Value);
	AActor* FindInteractableActor() const;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom()  const { return CameraBoom; }
	FORCEINLINE class UCameraComponent*   GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UInventoryComponent*      GetInventory()    const { return InventoryComponent; }
};