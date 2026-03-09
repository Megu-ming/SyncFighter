#include "SFChatWidget.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "SyncFighterClient/SFGameInstance.h"
#include "Internationalization/TextFormatter.h"
#include "SyncFighterClient/UI/Chat/SFChatMessage.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Types/SlateEnums.h"
#include "Kismet/GameplayStatics.h"

void USFChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->OnChatReceived.AddDynamic(this, &USFChatWidget::OnChatReceivedHandler);
	}
	if (ChatTextBox)
	{
		ChatTextBox->OnTextCommitted.AddDynamic(this, &USFChatWidget::OnChatTextCommitted);
	}

	ChatBox->SetVisibility(ESlateVisibility::Hidden);
	ChatTextBox->SetVisibility(ESlateVisibility::Hidden);
}

void USFChatWidget::OnChatReceivedHandler(int32 SenderID, FString Message)
{
	FFormatNamedArguments Args;

	Args.Add(TEXT("ID"), SenderID);
	Args.Add(TEXT("Msg"), FText::FromString(Message));

	FText FormattedResult = FText::Format(INVTEXT("[User {ID}] : {Msg}"), Args);

	if (ChatMessageClass)
	{
		UUserWidget* ChatMessageWidget = CreateWidget(this, ChatMessageClass);
		if (ChatMessageWidget)
		{
			USFChatMessage* ChatMessage = Cast<USFChatMessage>(ChatMessageWidget);
			ChatMessage->SetChatString(FormattedResult.ToString());

			ChatScrollBox->AddChild(ChatMessage);
			ChatScrollBox->ScrollToEnd();
			ChatBox->SetVisibility(ESlateVisibility::Visible);

			HideChatBox();
		}
	}
}

void USFChatWidget::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString TrimmedText = Text.ToString().TrimStartAndEnd();

	switch (CommitMethod)
	{
	case ETextCommit::OnEnter:
		if (TrimmedText.IsEmpty() == false)
		{
			USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
			GI->SendChatMessage(ChatTextBox->GetText().ToString());
			ChatTextBox->SetText(FText::FromString(TEXT("")));
		}
		break;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}
	ChatTextBox->SetVisibility(ESlateVisibility::Hidden);

	HideChatBox();
}

void USFChatWidget::OnChat()
{
	ChatBox->SetVisibility(ESlateVisibility::Visible);
	ChatTextBox->SetVisibility(ESlateVisibility::Visible);
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);
		ChatTextBox->SetUserFocus(PC);
	}
}

void USFChatWidget::HideChatBox()
{
	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &USFChatWidget::DoHideChatBox, 5.0f, false);
}

void USFChatWidget::DoHideChatBox()
{
	if (ChatBox)
	{
		ChatBox->SetVisibility(ESlateVisibility::Hidden);
	}
}
