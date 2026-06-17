#include "NpcActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/PanelWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "AnCharacter.h"

ANpcActor::ANpcActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    MeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
}

void ANpcActor::Interact_Implementation(AActor* Interactor)
{
    if (bIsInDialogue) return;

    CurrentInteractor = Interactor;
    CurrentLineIndex  = 0;
    bIsInDialogue     = true;

    if (AAnCharacter* Player = Cast<AAnCharacter>(Interactor))
        Player->EnterDialogue(this);

    StartDialogueCamera(Interactor);
    CreateDialogueWidget(Interactor);
    OnTalk(Interactor);

    if (DialogueLines.Num() > 0)
        ShowLine(0);
    else
        EndDialogue(Interactor);
}

EInteractableType ANpcActor::GetInteractableType_Implementation() const
{
    return EInteractableType::NPC;
}

void ANpcActor::AdvanceDialogue(AActor* Interactor)
{
    if (!bIsInDialogue || bWaitingForChoice) return;
    
    if (bEndsAfterLine)
    {
        bEndsAfterLine = false;
        EndDialogue(CurrentInteractor);
        return;
    }

    ShowLine(CurrentLineIndex + 1);
}

void ANpcActor::SelectChoice(int32 InChoiceIndex)
{
    if (!bIsInDialogue || !bWaitingForChoice) return;
    if (!DialogueLines.IsValidIndex(CurrentLineIndex)) return;

    const TArray<FDialogueChoice>& Choices = DialogueLines[CurrentLineIndex].Choices;
    if (!Choices.IsValidIndex(InChoiceIndex)) return;

    bWaitingForChoice = false;
    int32 Next = Choices[InChoiceIndex].NextLineIndex;

    if (Next < 0)
        EndDialogue(CurrentInteractor);
    else
        ShowLine(Next);
}

void ANpcActor::EndDialogue(AActor* Interactor)
{
    if (!bIsInDialogue) return;

    bIsInDialogue     = false;
    CurrentInteractor = nullptr;

    if (AAnCharacter* Player = Cast<AAnCharacter>(Interactor))
        Player->ExitDialogue();

    RemoveDialogueWidget();
    RestorePlayerCamera(Interactor);
    OnEndTalk(Interactor);
}

void ANpcActor::CreateDialogueWidget(AActor* Interactor)
{
    if (!DialogueWidgetClass) 
    {
        UE_LOG(LogTemp, Error, TEXT("[NPC] DialogueWidgetClass 미설정"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    DialogueWidget = CreateWidget<UUserWidget>(PC, DialogueWidgetClass);
    if (!DialogueWidget) return;

    if (FObjectProperty* Prop = FindFProperty<FObjectProperty>(
        DialogueWidget->GetClass(), TEXT("OwnerNpc")))
    {
        Prop->SetObjectPropertyValue_InContainer(DialogueWidget, this);
    }

    DialogueWidget->AddToViewport();
    
    DialogueWidget->WidgetTree->ForEachWidget([&](UWidget* Widget)
    {
        // ★ 트리 내 모든 위젯 이름 출력
    UE_LOG(LogTemp, Warning, TEXT("[NPC Widget] %s  (Class: %s)"),
        *Widget->GetFName().ToString(),
        *Widget->GetClass()->GetName());
        
        if (!TextName && Widget->GetFName() == TEXT("Text_Name"))
            TextName = Cast<UTextBlock>(Widget);

        if (!TextDialogue && Widget->GetFName() == TEXT("Text_Dialogue"))
            TextDialogue = Cast<UTextBlock>(Widget);

        if (!ChoiceBox && Widget->GetFName() == TEXT("ChoiceBox"))
            ChoiceBox = Cast<UPanelWidget>(Widget);
    });

    UE_LOG(LogTemp, Warning, TEXT("[NPC] TextName=%s  TextDialogue=%s  ChoiceBox=%s"),
        TextName     ? TEXT("OK") : TEXT("NULL"),
        TextDialogue ? TEXT("OK") : TEXT("NULL"),
        ChoiceBox    ? TEXT("OK") : TEXT("NULL"));
    
    PC->bShowMouseCursor = true;
    PC->SetInputMode(FInputModeGameAndUI());
}

void ANpcActor::RemoveDialogueWidget()
{
    if (DialogueWidget)
    {
        DialogueWidget->RemoveFromParent();
        DialogueWidget = nullptr;
        TextName       = nullptr;
        TextDialogue   = nullptr;
        ChoiceBox      = nullptr;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void ANpcActor::StartDialogueCamera(AActor* Interactor)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;
    
    FVector CamLocation = GetActorLocation() + GetActorRotation().RotateVector(DialogueCameraOffset);
    FRotator CamRotation = (GetActorLocation() - CamLocation).Rotation();

    FActorSpawnParameters Params;
    DialogueCameraActor = GetWorld()->SpawnActor<ACameraActor>(CamLocation, CamRotation, Params);

    if (DialogueCameraActor)
    {
        PC->SetViewTargetWithBlend(DialogueCameraActor, CameraBlendTime);
    }
}

void ANpcActor::RestorePlayerCamera(AActor* Interactor)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    if (APawn* Pawn = PC->GetPawn())
    {
        PC->SetViewTargetWithBlend(Pawn, CameraBlendTime);
    }

    if (DialogueCameraActor)
    {
        DialogueCameraActor->Destroy();
        DialogueCameraActor = nullptr;
    }
}

void ANpcActor::ShowLine(int32 Index)
{
    if (!DialogueLines.IsValidIndex(Index))
    {
        EndDialogue(CurrentInteractor);
        return;
    }

    CurrentLineIndex = Index;
    const FDialogueLine& Line = DialogueLines[Index];

    HideChoicesUI();
    UpdateDialogueUI(NpcName, Line.Text);
    OnDialogueLine(NpcName, Line.Text, Index);

    if (Line.Choices.Num() > 0)
    {
        bWaitingForChoice = true;
        ShowChoicesUI(Line.Choices);
        OnShowChoices(Line.Choices);
    }
    else
    {
        bWaitingForChoice = false;
        
        bEndsAfterLine = Line.bEndsDialogue;
    }
}

void ANpcActor::UpdateDialogueUI(const FText& Speaker, const FText& Line)
{
    if (TextName)     TextName->SetText(Speaker);
    if (TextDialogue) TextDialogue->SetText(Line);
}

void ANpcActor::ShowChoicesUI(const TArray<FDialogueChoice>& Choices)
{
    if (!ChoiceBox) return;
    if (!ChoiceButtonWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[NPC] ChoiceButtonWidgetClass 미설정"));
        return;
    }

    ChoiceBox->ClearChildren();
    ChoiceBox->SetVisibility(ESlateVisibility::Visible);

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    for (int32 i = 0; i < Choices.Num(); i++)
    {
        UUserWidget* BtnWidget = CreateWidget<UUserWidget>(PC, ChoiceButtonWidgetClass);
        if (!BtnWidget) continue;
        
        if (UButton* Btn = Cast<UButton>(BtnWidget->GetWidgetFromName(TEXT("ChoiceBtn"))))
        {
            UDialogueChoiceButton* Helper = NewObject<UDialogueChoiceButton>(this);
            Helper->OwnerNpc    = this;
            Helper->ChoiceIndex = i;
            Btn->OnClicked.AddDynamic(Helper, &UDialogueChoiceButton::OnClicked);
        }
        
        if (UTextBlock* Txt = Cast<UTextBlock>(BtnWidget->GetWidgetFromName(TEXT("ChoiceText"))))
        {
            Txt->SetText(Choices[i].ChoiceText);
        }

        ChoiceBox->AddChild(BtnWidget);
    }
}

void ANpcActor::HideChoicesUI()
{
    if (ChoiceBox)
    {
        ChoiceBox->ClearChildren();
        ChoiceBox->SetVisibility(ESlateVisibility::Collapsed);
    }
}