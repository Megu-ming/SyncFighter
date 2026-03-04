// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SFBlackhole.generated.h"

class ASFCharacter;
class UParticleSystemComponent;
class UParticleSystem;

UCLASS()
class SYNCFIGHTERCLIENT_API ASFBlackhole : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASFBlackhole();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// 블랙홀 시각 효과를 담당할 파티클 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UParticleSystemComponent* BlackHoleParticle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UParticleSystemComponent* EndBlackHoleParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* BeamParticleClass;

	FTimerHandle TickTimerHandle;
	int32 TickCount = 0;

	// 끌려온 희생자 명단
	UPROPERTY()
	TArray<ASFCharacter*> Victims;
	UPROPERTY()
	TMap<ASFCharacter*, class UParticleSystemComponent*> VictimBeams;

	UFUNCTION()
	void ApplyDamageAndCC();

	UFUNCTION()
	void EndBlackhole();

};
