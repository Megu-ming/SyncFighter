// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFDamageTextActor.generated.h"

class UWidgetComponent;

UCLASS()
class SYNCFIGHTERCLIENT_API ASFDamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASFDamageTextActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* DamageWidgetComp;

	void InitializeDamage(int32 DamageAmount);

};
