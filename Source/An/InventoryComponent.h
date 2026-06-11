#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemData.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const FItemData&, AddedItem);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(const FItemData& NewItem);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FItemData>& GetItems() const { return Items; }
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemQuantity(FName ItemID) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<FItemData> Items;
};