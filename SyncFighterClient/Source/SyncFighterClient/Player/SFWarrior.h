// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SFCharacter.h"
#include "SFWarrior.generated.h"

UCLASS()
class SYNCFIGHTERCLIENT_API ASFWarrior : public ASFCharacter
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

public:
	virtual void EndState() override;

	virtual void ProcessBasicAttack() override;
	virtual void ProcessSkillQ() override;
	virtual void ProcessSkillE() override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillEMontage;

#pragma region Q스킬 관련
	// Q 스킬 변수 및 함수
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillQMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* RecallWeaponMontage;

	// 남의 화면에서 쾅이 스킬을 쓸 때 재생할 함수
	void PlayRemoteSkillQ(FVector TargetLoc);

	// 애니메이션 노티파이에서 호출할 데미지 판정 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ApplySkillQDamage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<AActor> SkillQMagicClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSubclassOf<AActor> SkillIndicatorClass;

	UPROPERTY()
	AActor* CurrentIndicator;

	// 2. 조준 상태와 타겟 좌표 기억용 변수
	bool bIsAimingQ = false;
	FVector SkillQTargetLoc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsWeaponThrown = false;
	// 2. 바닥에 꽂힌 대검 액터를 기억해둘 포인터
	UPROPERTY()
	AActor* GroundedSword;
	// 3. 무기를 다시 회수하는 내부 로직
	void RecallWeapon();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndRecalled();
	// 무기를 회수 중일 때 True
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRecalled = false;
#pragma endregion

public:
#pragma region E스킬 관련
	// E스킬 몽타주에서 호출할 함수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillEIntroMontage;
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Teleport();

	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ApplySkillEDamage();
#pragma endregion

#pragma region 기본공격 관련
	// 기본공격
	UPROPERTY()
	TArray<AActor*> HitActors;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginMeleeAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckMeleeHit();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentMeleeDamage = 15;
#pragma endregion

	void CancelAiming();
};