#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UENUM(BlueprintType)
enum class EInteractableType : uint8
{
	Firefly		UMETA(DisplayName = "Firefly"),
	NPC			UMETA(DisplayName = "NPC"),
	Item		UMETA(DisplayName = "Item")
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class IInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	EInteractableType GetInteractableType() const;
};