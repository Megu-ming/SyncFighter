// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "SFChatWidget.generated.h"

class UBorder;
class UEditableTextBox;
class UScrollBox;
class USFChatMessage;

UCLASS()
class SYNCFIGHTERCLIENT_API USFChatWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	void OnChatReceivedHandler(int32 SenderID, FString Message);

	UFUNCTION()
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// Enter키 누르면 채팅창 열어주는 블루프린트에서 호출하는 함수
	UFUNCTION(BlueprintCallable)
	void OnChat();

private:
	// 5초 뒤 채팅창 비활성화 함수
	void HideChatBox();

	void DoHideChatBox();
	FTimerHandle HideTimerHandle;

public:
	UPROPERTY(meta = (BindWidget))
	UBorder* ChatBox;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatTextBox;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<USFChatMessage> ChatMessageClass;
};
