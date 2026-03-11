// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFLightning.generated.h"

class USphereComponent;

UCLASS()
class SYNCFIGHTERCLIENT_API ASFLightning : public AActor
{
	GENERATED_BODY()
	
public:	
	ASFLightning();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere)
	USphereComponent* SphereComponent;

	UFUNCTION()
	void Strike(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:
	UPROPERTY()
	TArray<AActor*> DamagedActors;
};
