// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFHUDWidget.generated.h"

class UTextBlock;

UCLASS()
class SYNCFIGHTERCLIENT_API USFHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MyScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnemyScoreText;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateTimer(int32 TimeInSeconds);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateMyScore(int32 Kills, int32 Deaths);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateEnemyScore(int32 Kills, int32 Deaths);
};
