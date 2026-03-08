// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFDamageTextWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class SYNCFIGHTERCLIENT_API USFDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FloatingAnim;

	void SetDamage(int32 Damage);
};
