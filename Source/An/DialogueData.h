#pragma once

#include "CoreMinimal.h"
#include "QuestData.h"
#include "DialogueData.generated.h"

USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText ChoiceText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 NextLineIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Quest")
	FQuestData QuestToAccept;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Quest")
	bool bOnlyShowWhenQuestNotStarted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Quest")
	bool bOnlyShowWhenQuestCompleted = false;
};

USTRUCT(BlueprintType)
struct FDialogueLine
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Text;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TArray<FDialogueChoice> Choices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bEndsDialogue = false;
};

USTRUCT(BlueprintType)
struct FQuestDialogueEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Quest")
	FName QuestID = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Quest")
	int32 ActiveStartIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Quest")
	int32 CompletedStartIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Quest")
	int32 FinishedStartIndex = -1;
};