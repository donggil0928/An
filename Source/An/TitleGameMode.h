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

	UFUNCTION(BlueprintCallable, Category = "Title")
	void StartGame();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Title|UI")
	TSubclassOf<class UUserWidget> TitleWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Title|UI")
	TSubclassOf<class UUserWidget> LoadingWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Title")
	FName GameLevelName = TEXT("GameLevel");

	UPROPERTY(EditDefaultsOnly, Category = "Title")
	float LoadingMinTime = 1.5f;

private:
	UPROPERTY()
	class UUserWidget* TitleWidget;

	UPROPERTY()
	class UUserWidget* LoadingWidget;

	void OpenGameLevel();
};
