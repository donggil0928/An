#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Blueprint/UserWidget.h"
#include "AnCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UInventoryComponent;
class UQuestComponent;
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
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 800.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PauseAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
	UQuestComponent* QuestComponent;

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

	UPROPERTY(EditAnywhere, Category = "Lantern|Socket", meta = (AllowPrivateAccess = "true"))
	FName LanternEquipSocket = TEXT("hand_r_socket");

	UPROPERTY(EditAnywhere, Category = "Lantern|Socket", meta = (AllowPrivateAccess = "true"))
	FName LanternDefaultSocket = TEXT("back_socket");

	UPROPERTY()
	class UMaterialInstanceDynamic* LanternMID;

	UPROPERTY()
	ANpcActor* ActiveNpc;

	bool bIsInDialogue = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lantern|State", meta = (AllowPrivateAccess = "true"))
	bool bIsState2 = true;

	UPROPERTY(EditAnywhere, Category = "Lantern|State", meta = (AllowPrivateAccess = "true"))
	float InactiveTimeout = 5.f;

	UPROPERTY(EditAnywhere, Category = "Lantern|Montage", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* EquipMontage;

	UPROPERTY(EditAnywhere, Category = "Lantern|Montage", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* UnequipMontage;

	UPROPERTY(EditAnywhere, Category = "Lantern|Montage", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* CollectMontage;

	bool bIsPlayingMontage  = false;
	bool bIsCollecting      = false;
	bool bIsLanternEquipped = false;
	bool bIsPaused          = false;

	FTimerHandle LanternUnequipTimerHandle;
	FTimerHandle IdleTimerHandle;

	void ResetActivityTimers();
	void OnLanternUnequipTimeout();
	void OnIdleTimeout();

	void SetABPIsState2(bool bValue);
	void PlayUnequipMontage();
	void AttachWeaponToSocket(FName SocketName);

	UFUNCTION()
	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnUnequipMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnCollectMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable, Category = "Lantern|Montage")
	void AnimNotify_EquipReady();

	UFUNCTION(BlueprintCallable, Category = "Lantern|Montage")
	void AnimNotify_UnequipReady();

	void OnPauseInput(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category = "Pause|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> PauseWidgetClass;

	UPROPERTY()
	class UUserWidget* PauseWidget;

	UFUNCTION()
	void OnInventoryUpdated(const FItemData& AddedItem);
	
	UPROPERTY(EditAnywhere, Category = "Interaction|HintUI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> InteractHintWidgetClass;

	UPROPERTY()
	UUserWidget* InteractHintWidget;

	UPROPERTY()
	AActor* LastHintActor;

	void ShowInteractHint(AActor* TargetActor);

	void HideInteractHint();

public:
	AAnCharacter();

	virtual void Tick(float DeltaSeconds) override;

	DECLARE_MULTICAST_DELEGATE(FOnLanternEquipped);
	FOnLanternEquipped OnLanternEquippedComplete;

	void EnterDialogue(ANpcActor* Npc);
	void ExitDialogue();

	UFUNCTION(BlueprintCallable, Category = "Pause")
	void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	FVector GetLanternLocation(FName SocketName = TEXT("lantern_socket")) const;

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	void AddLanternEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Lantern|State")
	bool IsLanternEquipped() const { return bIsLanternEquipped; }

	void PlayEquipMontage();
	void PlayCollectMontage();

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	bool IsCollecting() const { return bIsCollecting; }

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	float GetLanternEnergy() const { return LanternEnergy; }

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }

	UFUNCTION(BlueprintCallable, Category = "Lantern|State")
	bool GetIsState2() const { return bIsState2; }

	FORCEINLINE UInventoryComponent* GetInventory()      const { return InventoryComponent; }
	FORCEINLINE UQuestComponent*     GetQuestComponent() const { return QuestComponent; }

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void OnJumpOrAdvance(const FInputActionValue& Value);
	void AdvanceDialogue(const FInputActionValue& Value);
	void EndDialogueInput(const FInputActionValue& Value);
	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);
	AActor* FindInteractableActor() const;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom()   const { return CameraBoom; }
	FORCEINLINE class UCameraComponent*    GetFollowCamera()  const { return FollowCamera; }
};