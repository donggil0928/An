#include "FireflyActor.h"
#include "Components/StaticMeshComponent.h"
#include "AnCharacter.h"
#include "InventoryComponent.h"

AFireflyActor::AFireflyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	
	ItemData.ItemID      = FName("Firefly");
	ItemData.DisplayName = FText::FromString(TEXT("반딧불이"));
	ItemData.ItemType    = EItemType::Firefly;
	ItemData.Quantity    = 1;
}

void AFireflyActor::Interact_Implementation(AActor* Interactor)
{
	if (AAnCharacter* Player = Cast<AAnCharacter>(Interactor))
	{
		if (UInventoryComponent* Inv = Player->GetInventory())
		{
			Inv->AddItem(ItemData);
		}
		
		Player->AddLanternEnergy(LanternEnergyPerFirefly);
	}

	Destroy();
}

EInteractableType AFireflyActor::GetInteractableType_Implementation() const
{
	return EInteractableType::Firefly;
}