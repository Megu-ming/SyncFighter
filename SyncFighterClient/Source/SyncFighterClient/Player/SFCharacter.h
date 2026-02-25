// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SFCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle        UMETA(DisplayName = "기본"),
	Attacking   UMETA(DisplayName = "공격중/스킬시전중"), // Q, E 스킬 포함
	Dodging     UMETA(DisplayName = "회피중"),

	// 상태이상
	Stunned     UMETA(DisplayName = "기절"),
	Rooted      UMETA(DisplayName = "속박"),
	Airborne    UMETA(DisplayName = "공중에뜸"),
	Staggered   UMETA(DisplayName = "경직"),
	KnockedDown UMETA(DisplayName = "넘어짐"),

	Dead		UMETA(DisplayName = "사망")
};

UCLASS()
class SYNCFIGHTERCLIENT_API ASFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASFCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	ECharacterState CurrentState = ECharacterState::Idle;

	// 상태 변경 완료를 알리는 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void EndState();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

#pragma region InputAction
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockonAction;
	// ★ 신규 추가: Q, E, 회피 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillQAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillEAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;
#pragma endregion

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void BasicAttack(const FInputActionValue& Value);
	void SkillQ(const FInputActionValue& Value);
	void SkillE(const FInputActionValue& Value);
	void Dodge(const FInputActionValue& Value);

	void StartLockOn(const FInputActionValue& Value);
	void StopLockOn(const FInputActionValue& Value);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 1. 상태 관련 함수 (서버가 시킬 일들)
	virtual void ProcessBasicAttack();
	virtual void ProcessSkillQ();
	virtual void ProcessSkillE();
	virtual void ProcessDodge();

	virtual void ProcessDamage(int32 RemainingHP); // 피격/사망 처리
	virtual void ProcessRespawn(FVector NewLocation); // 부활 처리

	// 2. 변수들
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* HitMontage; // (나중에 쓸 것)

	// 3. UI 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "UI")
	class UWidgetComponent* HPBarComponent;

	// [Network Sync]
	FVector TargetLocation;   // 서버에서 받은 목표 위치
	FRotator TargetRotation;  // 서버에서 받은 목표 회전

	// 매 프레임 호출해서 위치를 맞추고 '가짜 속도'를 주입하는 함수
	void SyncTransform(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	ASFCharacter* LockOnTarget;

	// 기본공격 콤보
	int32 ComboIndex = 0;
	bool bHasNextComboInput = false;

	// 애니메이션 노티파이에서 호출할 콤보 체크 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckNextCombo();
};
