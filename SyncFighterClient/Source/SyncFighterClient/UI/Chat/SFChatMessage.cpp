#include "SFChatMessage.h"
#include "Components/TextBlock.h"

void USFChatMessage::SetChatString(FString Chat)
{
	MsgText->SetText(FText::FromString(Chat));
}
