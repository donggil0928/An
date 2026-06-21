#pragma once

#include "CoreMinimal.h"
#include "QuestData.generated.h"

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	NotStarted	UMETA(DisplayName = "미시작"),
	Active		UMETA(DisplayName = "진행중"),
	Completed	UMETA(DisplayName = "완료 가능"),
	Finished	UMETA(DisplayName = "종료")
};

UENUM(BlueprintType)
enum class EQuestConditionType : uint8
{
	CollectFirefly	UMETA(DisplayName = "반딧불이 수집"),
	CollectItem		UMETA(DisplayName = "아이템 수집"),
	None			UMETA(DisplayName = "없음")
};

USTRUCT(BlueprintType)
struct FQuestCondition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestConditionType ConditionType = EQuestConditionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName TargetItemID = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 RequiredCount = 1;
};

USTRUCT(BlueprintType)
struct FQuestData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName QuestID = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestCondition> Conditions;
};