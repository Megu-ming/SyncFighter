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
	BasicAttacking   UMETA(DisplayName = "공격중"),
	SkillAttacking   UMETA(DisplayName = "스킬시전중"),
	Jumping     UMETA(DisplayName = "점프중"),

	// 상태이상
	Stunned     UMETA(DisplayName = "기절"),
	Rooted      UMETA(DisplayName = "속박"),
	Staggered   UMETA(DisplayName = "경직"),

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
	virtual void BeginPlay() override;

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
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;
	// ★ 신규 추가: Q, E, 회피 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillQAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SkillEAction;

#pragma endregion

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void BasicAttack(const FInputActionValue& Value);
	void SkillQ(const FInputActionValue& Value);
	void SkillE(const FInputActionValue& Value);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 1. 상태 관련 함수 (서버가 시킬 일들)
	virtual void ProcessBasicAttack();
	virtual void ProcessSkillQ();
	virtual void ProcessSkillE();

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

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> CrosshairWidgetClass; // 에디터에서 넣을 빈칸

	UPROPERTY()
	class UUserWidget* CrosshairWidget;

	// [Network Sync]
	FVector TargetLocation;   // 서버에서 받은 목표 위치
	FRotator TargetRotation;  // 서버에서 받은 목표 회전

	// 매 프레임 호출해서 위치를 맞추고 '가짜 속도'를 주입하는 함수
	void SyncTransform(float DeltaTime);

	// 기본공격 콤보
	int32 ComboIndex = 0;
	bool bHasNextComboInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxComboCount = 3;

	// 애니메이션 노티파이에서 호출할 콤보 체크 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckNextCombo();
};
