#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestData.h"
#include "QuestComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, FName, QuestID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AcceptQuest(const FQuestData& InQuest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void EvaluateQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void EvaluateAllActiveQuests();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool FinishQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	EQuestState GetQuestState(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestActive(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestCompleted(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestFinished(FName QuestID) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "Quest")
	TMap<FName, EQuestState> QuestStates;
	
	UPROPERTY()
	TMap<FName, FQuestData> QuestDataMap;
	
	bool CheckConditions(const FQuestData& Quest) const;
};