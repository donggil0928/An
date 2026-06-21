#include "QuestComponent.h"
#include "InventoryComponent.h"
#include "AnCharacter.h"

UQuestComponent::UQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UQuestComponent::AcceptQuest(const FQuestData& InQuest)
{
	if (InQuest.QuestID.IsNone()) return false;
	
	if (QuestStates.Contains(InQuest.QuestID)) return false;

	QuestStates.Add(InQuest.QuestID, EQuestState::Active);
	QuestDataMap.Add(InQuest.QuestID, InQuest);

	UE_LOG(LogTemp, Log, TEXT("[Quest] 퀘스트 수락: %s"), *InQuest.QuestID.ToString());
	OnQuestStateChanged.Broadcast(InQuest.QuestID);

	EvaluateQuest(InQuest.QuestID);
	return true;
}

void UQuestComponent::EvaluateQuest(FName QuestID)
{
	EQuestState* State = QuestStates.Find(QuestID);
	if (!State || *State != EQuestState::Active) return;

	const FQuestData* Data = QuestDataMap.Find(QuestID);
	if (!Data) return;

	if (CheckConditions(*Data))
	{
		*State = EQuestState::Completed;
		UE_LOG(LogTemp, Log, TEXT("[Quest] 조건 달성: %s"), *QuestID.ToString());
		OnQuestStateChanged.Broadcast(QuestID);
	}
}

void UQuestComponent::EvaluateAllActiveQuests()
{
	TArray<FName> Keys;
	QuestStates.GetKeys(Keys);
	for (const FName& ID : Keys)
	{
		if (QuestStates[ID] == EQuestState::Active)
			EvaluateQuest(ID);
	}
}

bool UQuestComponent::FinishQuest(FName QuestID)
{
	EQuestState* State = QuestStates.Find(QuestID);
	if (!State || *State != EQuestState::Completed) return false;

	*State = EQuestState::Finished;
	UE_LOG(LogTemp, Log, TEXT("[Quest] 퀘스트 완료: %s"), *QuestID.ToString());
	OnQuestStateChanged.Broadcast(QuestID);
	return true;
}

EQuestState UQuestComponent::GetQuestState(FName QuestID) const
{
	const EQuestState* State = QuestStates.Find(QuestID);
	return State ? *State : EQuestState::NotStarted;
}

bool UQuestComponent::IsQuestActive(FName QuestID) const
{
	return GetQuestState(QuestID) == EQuestState::Active;
}

bool UQuestComponent::IsQuestCompleted(FName QuestID) const
{
	const EQuestState S = GetQuestState(QuestID);
	return S == EQuestState::Completed || S == EQuestState::Finished;
}

bool UQuestComponent::IsQuestFinished(FName QuestID) const
{
	return GetQuestState(QuestID) == EQuestState::Finished;
}

bool UQuestComponent::CheckConditions(const FQuestData& Quest) const
{
	if (Quest.Conditions.Num() == 0) return true;
	
	AAnCharacter* Owner = Cast<AAnCharacter>(GetOwner());
	UInventoryComponent* Inv = Owner ? Owner->GetInventory() : nullptr;

	for (const FQuestCondition& Cond : Quest.Conditions)
	{
		switch (Cond.ConditionType)
		{
		case EQuestConditionType::CollectFirefly:
		{
			FName FireflyID = Cond.TargetItemID.IsNone() ? FName("Firefly") : Cond.TargetItemID;
			int32 Count = Inv ? Inv->GetItemQuantity(FireflyID) : 0;
			if (Count < Cond.RequiredCount) return false;
			break;
		}
		case EQuestConditionType::CollectItem:
		{
			int32 Count = Inv ? Inv->GetItemQuantity(Cond.TargetItemID) : 0;
			if (Count < Cond.RequiredCount) return false;
			break;
		}
		default:
			break;
		}
	}
	return true;
}