// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SFCharacter.h"
#include "SFMage.generated.h"

UCLASS()
class SYNCFIGHTERCLIENT_API ASFMage : public ASFCharacter
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;

public:
	virtual void EndState() override;

	virtual void ProcessBasicAttack() override;
	virtual void ProcessSkillQ() override;
	void PlayRemoteSkillQ(FVector TargetLoc);
	virtual void ProcessSkillE() override;
	void PlayRemoteSkillE(FVector TargetLoc);

	// --- [투사체(마법) 블루프린트 할당 칸] ---
	UPROPERTY(EditAnywhere, Category = "Combat|Magic")
	TSubclassOf<class AActor> BasicFireballClass;

	UPROPERTY(EditAnywhere, Category = "Combat|Magic")
	TSubclassOf<class AActor> SkillQMagicClass; // 얼음 속박 or 거대 화염구

	// --- [마법 시전 애니메이션 칸] ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillQMontage;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<AActor> SkillIndicatorClass;

	// 2. 현재 맵에 띄워둔 장판 액터를 기억할 포인터
	UPROPERTY()
	AActor* CurrentIndicator;

	// 3. 조준 중인지 확인하는 깃발(Flag)
	bool bIsAimingQ = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillEMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Magic")
	TSubclassOf<class AActor> SkillEMagicClass;

	FVector SkillQTargetLoc;
	FVector SkillETargetLoc;

	// 애니메이션 노티파이에서 호출할 실제 액터 스폰 함수
	UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
	void SpawnSkillQMagic();

	UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
	void SpawnSkillEMagic();

public:
	UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
	void FireMagic(FName SocketName);

	void CancelAiming();
};
