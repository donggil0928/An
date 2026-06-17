#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "DialogueData.h"
#include "DialogueChoiceButton.h"
#include "NpcActor.generated.h"

UCLASS()
class ANpcActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ANpcActor();
	UPROPERTY()
	ANpcActor* OwnerNpc;

	int32 ChoiceIndex;

	UFUNCTION()
	void OnClicked()
	{
		if (OwnerNpc)
			OwnerNpc->SelectChoice(ChoiceIndex);
	}
	
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual EInteractableType GetInteractableType_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void AdvanceDialogue(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	void SelectChoice(int32 InChoiceIndex);
	
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void EndDialogue(AActor* Interactor);

protected:
	UPROPERTY(VisibleAnywhere, Category = "NPC")
	class USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FText NpcName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
	TArray<FDialogueLine> DialogueLines;

	UPROPERTY(EditAnywhere, Category = "NPC|UI")
	TSubclassOf<class UUserWidget> DialogueWidgetClass;

	UPROPERTY(EditAnywhere, Category = "NPC|UI")
	TSubclassOf<class UUserWidget> ChoiceButtonWidgetClass;
	
	UPROPERTY(EditAnywhere, Category = "NPC|Camera")
	FVector DialogueCameraOffset = FVector(-180.f, 120.f, 80.f);

	UPROPERTY(EditAnywhere, Category = "NPC|Camera")
	float CameraBlendTime = 0.6f;
	
	UPROPERTY(BlueprintReadOnly, Category = "NPC|Dialogue")
	int32 CurrentLineIndex = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "NPC|Dialogue")
	bool bIsInDialogue = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Dialogue")
	bool bWaitingForChoice = false;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
	void OnDialogueLine(const FText& SpeakerName, const FText& Line, int32 LineIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Dialogue")
	void OnShowChoices(const TArray<FDialogueChoice>& Choices);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
	void OnTalk(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
	void OnEndTalk(AActor* Interactor);

private:
	UPROPERTY()
	class ACameraActor* DialogueCameraActor;

	UPROPERTY()
	AActor* CurrentInteractor;

	UPROPERTY()
	class UUserWidget* DialogueWidget;

	class UTextBlock* TextName;
	class UTextBlock* TextDialogue;
	class UPanelWidget* ChoiceBox;

	bool bEndsAfterLine = false;
	
	void ShowLine(int32 Index);
	void UpdateDialogueUI(const FText& Speaker, const FText& Line);
	void ShowChoicesUI(const TArray<FDialogueChoice>& Choices);
	void HideChoicesUI();
	void CreateDialogueWidget(AActor* Interactor);
	void RemoveDialogueWidget();
	
	void StartDialogueCamera(AActor* Interactor);
	void RestorePlayerCamera(AActor* Interactor);
};