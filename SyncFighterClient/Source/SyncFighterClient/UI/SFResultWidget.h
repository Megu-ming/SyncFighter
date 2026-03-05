// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFResultWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class SYNCFIGHTERCLIENT_API USFResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	// ★ 결과 텍스트 (VICTORY or DEFEAT)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FinalScoreText;

	// ★ 메인 화면으로 돌아가기 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;

	// 게임 오버 시 네트워크 액터가 호출해줄 함수
	UFUNCTION(BlueprintCallable, Category = "Result")
	void SetResultInfo(bool bIsWinner, int32 MyKills, int32 MyDeaths);

	UFUNCTION()
	void OnReturnButtonClicked();
};
