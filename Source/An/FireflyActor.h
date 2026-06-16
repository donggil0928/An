#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "ItemData.h"
#include "FireflyActor.generated.h"

UCLASS()
class AFireflyActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AFireflyActor();

	virtual void Tick(float DeltaSeconds) override;

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual EInteractableType GetInteractableType_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Firefly")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Firefly")
	FItemData ItemData;

	UPROPERTY(EditAnywhere, Category = "Firefly")
	float LanternEnergyPerFirefly = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "Firefly")
	float FlySpeed = 400.f;
	
	UPROPERTY(EditAnywhere, Category = "Firefly")
	float ArrivalThreshold = 15.f;
	
	UPROPERTY(EditAnywhere, Category = "Firefly")
	FName LanternSocketName = TEXT("lantern_socket");

private:
	bool bFlying = false;
	
	UPROPERTY()
	class AAnCharacter* TargetPlayer;
};