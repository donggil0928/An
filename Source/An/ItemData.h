#pragma once

#include "CoreMinimal.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Firefly		UMETA(DisplayName = "반딧불이"),
	QuestItem	UMETA(DisplayName = "퀘스트 아이템"),
	Misc		UMETA(DisplayName = "기타")
};

USTRUCT(BlueprintType)
struct FItemData
{
	GENERATED_BODY()

	/** 아이템 고유 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID;

	/** 화면에 표시할 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText DisplayName;

	/** 아이템 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType = EItemType::Misc;

	/** 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity = 1;

	/** 인벤토리 아이콘 (블루프린트에서 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* Icon = nullptr;
};
