#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriggerOverlayActor.generated.h"

UCLASS()
class ATriggerOverlayActor : public AActor
{
	GENERATED_BODY()

public:
	ATriggerOverlayActor();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	class UBoxComponent* TriggerBox;
	
	UPROPERTY(EditAnywhere, Category = "Trigger|UI")
	TSubclassOf<class UUserWidget> OverlayWidgetClass;
	
	UPROPERTY(EditAnywhere, Category = "Trigger|UI")
	float FadeInDuration = 1.f;

	UPROPERTY(EditAnywhere, Category = "Trigger|UI")
	float DisplayDuration = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "Trigger|UI")
	float FadeOutDuration = 1.f;
	
	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bTriggerOnce = true;

	UPROPERTY(EditAnywhere, Category = "Trigger|UI")
	int32 WidgetZOrder = 5;

private:
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp,
	                       AActor* OtherActor,
	                       UPrimitiveComponent* OtherComp,
	                       int32 OtherBodyIndex,
	                       bool bFromSweep,
	                       const FHitResult& SweepResult);

	void StartFadeIn();
	void StartHold();
	void StartFadeOut();
	void FinishOverlay();
	
	enum class EFadeState : uint8
	{
		Idle,
		FadeIn,
		Hold,
		FadeOut
	};
	EFadeState FadeState = EFadeState::Idle;

	float FadeElapsed = 0.f;

	FTimerHandle HoldTimerHandle;
	FTimerHandle FadeOutTimerHandle;

	UPROPERTY()
	class UUserWidget* ActiveWidget = nullptr;

	bool bHasTriggered = false;
};