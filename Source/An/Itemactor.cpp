#include "ItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "AnCharacter.h"
#include "InventoryComponent.h"

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
}

void AItemActor::Interact_Implementation(AActor* Interactor)
{
	if (AAnCharacter* Player = Cast<AAnCharacter>(Interactor))
	{
		if (UInventoryComponent* Inv = Player->GetInventory())
		{
			Inv->AddItem(ItemData);
		}
	}

	Destroy();
}

EInteractableType AItemActor::GetInteractableType_Implementation() const
{
	return EInteractableType::Item;
}