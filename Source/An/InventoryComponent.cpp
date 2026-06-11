#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddItem(const FItemData& NewItem)
{
	for (FItemData& Existing : Items)
	{
		if (Existing.ItemID == NewItem.ItemID)
		{
			Existing.Quantity += NewItem.Quantity;
			OnInventoryUpdated.Broadcast(Existing);
			UE_LOG(LogTemp, Log, TEXT("[인벤토리] %s 수량 증가 → %d"), *NewItem.DisplayName.ToString(), Existing.Quantity);
			return;
		}
	}
	
	Items.Add(NewItem);
	OnInventoryUpdated.Broadcast(NewItem);
	UE_LOG(LogTemp, Log, TEXT("[인벤토리] %s 추가"), *NewItem.DisplayName.ToString());
}

int32 UInventoryComponent::GetItemQuantity(FName ItemID) const
{
	for (const FItemData& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			return Item.Quantity;
		}
	}
	return 0;
}