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
	
	UPROPERTY(EditAnywhere, Category = "Portal|Cinematic")
	class ULevelSequence* OpenSequence;

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

	void PlayOpenCinematic();
};