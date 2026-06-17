#include "FireflyActor.h"
#include "Components/StaticMeshComponent.h"
#include "AnCharacter.h"
#include "InventoryComponent.h"

AFireflyActor::AFireflyActor()
{
	PrimaryActorTick.bCanEverTick  = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

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
	AAnCharacter* Player = Cast<AAnCharacter>(Interactor);
	if (!Player) return;

	TargetPlayer = Player;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (Player->IsLanternEquipped())
	{
		Player->PlayCollectMontage();
		bFlying = true;
		SetActorTickEnabled(true);
		return;
	}
	
	bWaitingForEquip = true;
	Player->OnLanternEquippedComplete.AddUObject(this, &AFireflyActor::OnLanternReady);
	Player->PlayEquipMontage();
}

void AFireflyActor::OnLanternReady()
{
	if (!bWaitingForEquip || !TargetPlayer) return;

	bWaitingForEquip = false;
	TargetPlayer->OnLanternEquippedComplete.RemoveAll(this);

	TargetPlayer->PlayCollectMontage();
	bFlying = true;
	SetActorTickEnabled(true);
}

void AFireflyActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bFlying || !TargetPlayer) return;

	//if (!TargetPlayer->IsLanternEquipped()) return;
	
	const FVector TargetLocation = TargetPlayer->GetLanternLocation(LanternSocketName);
	const FVector CurrentLocation = GetActorLocation();
	const FVector ToTarget = TargetLocation - CurrentLocation;
	const float   Distance = ToTarget.Size();
	
	if (Distance <= ArrivalThreshold)
	{
		if (UInventoryComponent* Inv = TargetPlayer->GetInventory())
		{
			Inv->AddItem(ItemData);
		}
		
		TargetPlayer->AddLanternEnergy(LanternEnergyPerFirefly);

		Destroy();
		return;
	}
	
	const FVector MoveDir   = ToTarget / Distance;
	const FVector NewLocation = CurrentLocation + MoveDir * FlySpeed * DeltaSeconds;
	SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

EInteractableType AFireflyActor::GetInteractableType_Implementation() const
{
	return EInteractableType::Firefly;
}