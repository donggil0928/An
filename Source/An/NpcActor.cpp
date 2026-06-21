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
#include "QuestComponent.h"

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
    CurrentLineIndex  = ResolveStartIndex();
    bIsInDialogue     = true;

    if (AAnCharacter* Player = Cast<AAnCharacter>(Interactor))
        Player->EnterDialogue(this);

    StartDialogueCamera(Interactor);
    CreateDialogueWidget(Interactor);
    OnTalk(Interactor);

    if (DialogueLines.Num() > 0)
        ShowLine(CurrentLineIndex);
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
    
    if (!ActiveChoices.IsValidIndex(InChoiceIndex)) return;

    bWaitingForChoice = false;

    const FDialogueChoice& Choice = ActiveChoices[InChoiceIndex];
    
    HandleQuestAccept(Choice);
    
    HandleQuestFinish(Choice);

    int32 Next = Choice.NextLineIndex;
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

int32 ANpcActor::ResolveStartIndex() const
{
    if (QuestDialogueEntries.Num() == 0) return 0;

    AAnCharacter* Player = Cast<AAnCharacter>(CurrentInteractor);
    UQuestComponent* QC  = Player ? Player->FindComponentByClass<UQuestComponent>() : nullptr;
    if (!QC) return 0;
    
    for (const FQuestDialogueEntry& Entry : QuestDialogueEntries)
    {
        if (Entry.QuestID.IsNone()) continue;

        const EQuestState State = QC->GetQuestState(Entry.QuestID);

        switch (State)
        {
        case EQuestState::Finished:
            if (Entry.FinishedStartIndex >= 0)
                return Entry.FinishedStartIndex;
            break;

        case EQuestState::Completed:
            if (Entry.CompletedStartIndex >= 0)
                return Entry.CompletedStartIndex;
            if (Entry.ActiveStartIndex >= 0)
                return Entry.ActiveStartIndex;
            break;

        case EQuestState::Active:
            if (Entry.ActiveStartIndex >= 0)
                return Entry.ActiveStartIndex;
            break;

        default:
            break;
        }
    }

    return 0;
}

TArray<FDialogueChoice> ANpcActor::FilterChoices(const TArray<FDialogueChoice>& InChoices) const
{
    AAnCharacter* Player = Cast<AAnCharacter>(CurrentInteractor);
    UQuestComponent* QC  = Player ? Player->FindComponentByClass<UQuestComponent>() : nullptr;

    TArray<FDialogueChoice> Result;
    for (const FDialogueChoice& Choice : InChoices)
    {
        if (Choice.bOnlyShowWhenQuestNotStarted)
        {
            if (!Choice.QuestToAccept.QuestID.IsNone())
            {
                EQuestState S = QC ? QC->GetQuestState(Choice.QuestToAccept.QuestID)
                                   : EQuestState::NotStarted;
                if (S != EQuestState::NotStarted) continue;
            }
        }
        
        if (Choice.bOnlyShowWhenQuestCompleted)
        {
            if (!Choice.QuestToAccept.QuestID.IsNone())
            {
                EQuestState S = QC ? QC->GetQuestState(Choice.QuestToAccept.QuestID)
                                   : EQuestState::NotStarted;
                if (S != EQuestState::Completed) continue;
            }
        }

        Result.Add(Choice);
    }
    return Result;
}

void ANpcActor::HandleQuestAccept(const FDialogueChoice& Choice)
{
    if (Choice.QuestToAccept.QuestID.IsNone()) return;
    // Completed 전용 선택지는 수락이 아닌 보고 처리이므로 건너뜀
    if (Choice.bOnlyShowWhenQuestCompleted) return;

    AAnCharacter* Player = Cast<AAnCharacter>(CurrentInteractor);
    UQuestComponent* QC  = Player ? Player->FindComponentByClass<UQuestComponent>() : nullptr;
    if (!QC) return;

    if (QC->AcceptQuest(Choice.QuestToAccept))
    {
        UE_LOG(LogTemp, Log, TEXT("[NPC] 퀘스트 수락됨: %s"),
            *Choice.QuestToAccept.QuestID.ToString());
    }
}

void ANpcActor::HandleQuestFinish(const FDialogueChoice& Choice)
{
    if (!Choice.bOnlyShowWhenQuestCompleted) return;
    if (Choice.QuestToAccept.QuestID.IsNone()) return;

    AAnCharacter* Player = Cast<AAnCharacter>(CurrentInteractor);
    UQuestComponent* QC  = Player ? Player->FindComponentByClass<UQuestComponent>() : nullptr;
    if (!QC) return;

    if (QC->FinishQuest(Choice.QuestToAccept.QuestID))
    {
        UE_LOG(LogTemp, Log, TEXT("[NPC] 퀘스트 완료 보고됨: %s"),
            *Choice.QuestToAccept.QuestID.ToString());
    }
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
        DialogueWidget  = nullptr;
        TextName        = nullptr;
        TextDialogue    = nullptr;
        ChoiceBox       = nullptr;
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

    FVector   CamLocation = GetActorLocation() + GetActorRotation().RotateVector(DialogueCameraOffset);
    FRotator  CamRotation = (GetActorLocation() - CamLocation).Rotation();

    FActorSpawnParameters Params;
    DialogueCameraActor = GetWorld()->SpawnActor<ACameraActor>(CamLocation, CamRotation, Params);

    if (DialogueCameraActor)
        PC->SetViewTargetWithBlend(DialogueCameraActor, CameraBlendTime);
}

void ANpcActor::RestorePlayerCamera(AActor* Interactor)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    if (APawn* Pawn = PC->GetPawn())
        PC->SetViewTargetWithBlend(Pawn, CameraBlendTime);

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
        ActiveChoices = FilterChoices(Line.Choices);

        if (ActiveChoices.Num() > 0)
        {
            bWaitingForChoice = true;
            ShowChoicesUI(ActiveChoices);
            OnShowChoices(ActiveChoices);
        }
        else
        {
            bWaitingForChoice = false;
            bEndsAfterLine    = Line.bEndsDialogue;
        }
    }
    else
    {
        bWaitingForChoice = false;
        bEndsAfterLine    = Line.bEndsDialogue;
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