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

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
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

public:
	AAnCharacter();
	
	void EnterDialogue(ANpcActor* Npc);
	
	void ExitDialogue();

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void Interact(const FInputActionValue& Value);

	void OnJumpOrAdvance(const FInputActionValue& Value);

	void AdvanceDialogue(const FInputActionValue& Value);

	void EndDialogueInput(const FInputActionValue& Value);

	AActor* FindInteractableActor() const;

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent*   GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UInventoryComponent*      GetInventory() const { return InventoryComponent; }

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	void AddLanternEnergy(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Lantern")
	float GetLanternEnergy() const { return LanternEnergy; }

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }
};