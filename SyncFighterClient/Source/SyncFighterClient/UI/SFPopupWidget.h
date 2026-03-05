// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFPopupWidget.generated.h"

class UButton;

UCLASS()
class SYNCFIGHTERCLIENT_API USFPopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UButton* YesButton;

	UPROPERTY(meta = (BindWidget))
	UButton* NoButton;

	UFUNCTION()
	void OnYesClicked();

	UFUNCTION()
	void OnNoClicked();

public:
	void CacheInfo(FString ID, FString PW);

	FString NewID;
	FString NewPW;
};
