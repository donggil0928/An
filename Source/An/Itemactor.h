#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "ItemData.h"
#include "ItemActor.generated.h"
 
UCLASS()
class AItemActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
 
public:
	AItemActor();
 
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual EInteractableType GetInteractableType_Implementation() const override;
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Item")
	class UStaticMeshComponent* MeshComp;
 
	/** 블루프린트에서 아이템 데이터 지정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemData ItemData;
};