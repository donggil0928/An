#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TitleGameMode.generated.h"

UCLASS()
class ATitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATitleGameMode();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Title")
	void StartGame();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Title|UI")
	TSubclassOf<class UUserWidget> TitleWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Title|UI")
	TSubclassOf<class UUserWidget> LoadingWidgetClass;


	UPROPERTY(EditDefaultsOnly, Category = "Title")
	TSoftObjectPtr<UWorld> GameLevel;
	
	UPROPERTY(EditDefaultsOnly, Category = "Title")
	float LoadingMinTime = 3.5f;

private:
	UPROPERTY()
	class UUserWidget* TitleWidget;

	UPROPERTY()
	class ULoadingWidget* LoadingWidget;

	UPROPERTY()
	class ULevelStreamingDynamic* StreamingLevel;

	bool bIsLoading      = false;
	bool bLevelLoaded    = false;
	bool bMinTimeElapsed = false;

	FTimerHandle MinTimeHandle;

	void OnMinTimeElapsed();
	void TryOpenGameLevel();
};