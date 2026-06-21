#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "PortalActor.generated.h"

UCLASS()
class APortalActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	APortalActor();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual EInteractableType GetInteractableType_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Portal")
	class USkeletalMeshComponent* MeshComp;
	
	UPROPERTY(EditAnywhere, Category = "Portal")
	float RequiredEnergy = 100.f;

	/** ★ 테스트용: 켜면 에너지 조건 무시하고 즉시 활성화 */
	UPROPERTY(EditAnywhere, Category = "Portal|Debug")
	bool bIgnoreEnergyForTest = true;
	
	UPROPERTY(EditAnywhere, Category = "Portal|Cinematic")
	class ULevelSequence* OpenSequence;
	
	UPROPERTY(EditAnywhere, Category = "Portal|Cinematic")
	class UAnimMontage* OpenMontage;
	
	UPROPERTY(EditAnywhere, Category = "Portal|End")
	TSubclassOf<class UUserWidget> EndWidgetClass;
	
	UPROPERTY(EditAnywhere, Category = "Portal|End")
	float EndWidgetDelay = 5.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Portal")
	bool bIsActivated = false;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Portal")
	void OnPortalOpen();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Portal")
	void OnPortalLocked(float CurrentEnergy, float Required);
	
	
	
	UFUNCTION()
	void OnSequenceFinished();

private:
	UPROPERTY()
	class ULevelSequencePlayer* SequencePlayer;

	UPROPERTY()
	AActor* CurrentInteractor;

	FTimerHandle EndWidgetTimerHandle;	

	void PlayOpenCinematic();
	void PlayOpenMontage();
	void StartEndWidgetTimer();
	void ShowEndWidget();
};