// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFMenuWidget.generated.h"

class UButton;

UCLASS()
class SYNCFIGHTERCLIENT_API USFMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeBtn; // 게임으로 돌아가기

	UPROPERTY(meta = (BindWidget))
	UButton* QuitBtn;   // 바탕화면으로 종료

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnQuitClicked();
};
