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
	Attacking   UMETA(DisplayName = "공격중"),
	Hit         UMETA(DisplayName = "피격중"),
	Jumping     UMETA(DisplayName = "점프중"),
	KnockedDown UMETA(DisplayName = "넘어진상태"),
	Dead		UMETA(DisplayName = "사망"),
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
	void EndState();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockonAction;

protected:
	// 입력 처리 함수들
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Attack(const FInputActionValue& Value); // 공격

	void StartLockOn(const FInputActionValue& Value);
	void StopLockOn(const FInputActionValue& Value);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 1. 상태 관련 함수 (서버가 시킬 일들)
	void ProcessAttack(); // 공격 모션 재생
	void ProcessDamage(int32 RemainingHP); // 피격/사망 처리
	void ProcessRespawn(FVector NewLocation); // 부활 처리

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
};
