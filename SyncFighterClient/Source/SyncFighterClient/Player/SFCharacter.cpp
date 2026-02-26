#include "SFCharacter.h"
#include "Components/WidgetComponent.h"
#include "SyncFighterClient/UI/SFHPBarWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SyncFighterClient/SFGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ASFCharacter::ASFCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(25.f, 88.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; // Character look Camera direction
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// 1. 스프링암 컴포넌트 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.0f, 70.0f, 50.0f);

	// 2. 위젯 컴포넌트 설정 (이사 옴)
	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComponent->SetupAttachment(GetRootComponent());
	HPBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	HPBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarComponent->SetDrawSize(FVector2D(150.0f, 20.0f));

	// 3. 카메라 컴포넌트 설정
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ASFCharacter::EndState()
{
	CurrentState = ECharacterState::Idle;

	ComboIndex = 0;
	bHasNextComboInput = false;
}

void ASFCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (IsLocallyControlled() && CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport(); // 화면에 부착!
		}
	}
}

void ASFCharacter::Move(const FInputActionValue& Value)
{
	if (CurrentState != ECharacterState::Idle 
		&& CurrentState != ECharacterState::Jumping
		&& CurrentState != ECharacterState::BasicAttacking)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASFCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASFCharacter::BasicAttack(const FInputActionValue& Value)
{
	if (CurrentState != ECharacterState::Idle 
		&& CurrentState != ECharacterState::Rooted
		&& CurrentState != ECharacterState::BasicAttacking) return;

	ProcessBasicAttack();
}

void ASFCharacter::SkillQ(const FInputActionValue& Value)
{
	if (CurrentState != ECharacterState::Idle) return; // 기본 상태에서만 가능

	CurrentState = ECharacterState::SkillAttacking;
	ProcessSkillQ();

	// TODO: 서버로 Q스킬 패킷 전송
}

void ASFCharacter::SkillE(const FInputActionValue& Value)
{
	if (CurrentState != ECharacterState::Idle) return; // 기본 상태에서만 가능

	CurrentState = ECharacterState::SkillAttacking;
	ProcessSkillE();

	// TODO: 서버로 E스킬 패킷 전송
}

void ASFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		UE_LOG(LogTemp, Warning, TEXT("ASFCharacter::SetupPlayerInputComponent Binding Start"));
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASFCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASFCharacter::Look);
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASFCharacter::BasicAttack);
		EnhancedInputComponent->BindAction(SkillQAction, ETriggerEvent::Started, this, &ASFCharacter::SkillQ);
		EnhancedInputComponent->BindAction(SkillEAction, ETriggerEvent::Started, this, &ASFCharacter::SkillE);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASFCharacter::ProcessBasicAttack()
{
	if (CurrentState == ECharacterState::Idle || CurrentState == ECharacterState::Rooted)
	{
		CurrentState = ECharacterState::BasicAttacking;
		ComboIndex = 1;
		bHasNextComboInput = false;

		// 몽타주의 "Combo1" 섹션부터 재생
		if (AttackMontage)
		{
			PlayAnimMontage(AttackMontage, 1.0f, FName("Combo1"));

			USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
			if (GI)
			{
				PacketPlayerAttack AttackPacket;
				AttackPacket.Size = sizeof(PacketPlayerAttack);
				AttackPacket.Id = C_TO_S_PLAYER_ATTACK;
				AttackPacket.PlayerID = GI->MyPlayerID; // 내 ID

				GI->SendPacket(&AttackPacket, sizeof(AttackPacket));
				UE_LOG(LogTemp, Log, TEXT("Attack Packet Sent!"));
			}
		}
	}
	else if (CurrentState == ECharacterState::BasicAttacking)
	{
		if (ComboIndex < MaxComboCount)
		{
			bHasNextComboInput = true;
			UE_LOG(LogTemp, Log, TEXT("[캐릭터] 다음 콤보 예약됨! (현재: %d타 / 최대: %d타)"), ComboIndex, MaxComboCount);
		}
	}
}

void ASFCharacter::ProcessSkillQ()
{
}

void ASFCharacter::ProcessSkillE()
{
}

void ASFCharacter::ProcessDodge()
{
	// 임시
	UE_LOG(LogTemp, Log, TEXT("회피 혹은 기상 시전!"));
	EndState();
}

void ASFCharacter::ProcessDamage(int32 RemainingHP)
{
	// 1. UI 업데이트
	if (HPBarComponent)
	{
		USFHPBarWidget* HPWidget = Cast<USFHPBarWidget>(HPBarComponent->GetUserWidgetObject());
		if (HPWidget)
		{
			HPWidget->UpdateHP((float)RemainingHP, 100.0f);
		}
	}

	// 2. 사망 처리
	if (RemainingHP <= 0 && CurrentState != ECharacterState::Dead)
	{
		CurrentState = ECharacterState::Dead;
		if (DeathMontage) PlayAnimMontage(DeathMontage);

		// 캡슐 충돌 끄기 (시체 위로 걸어다닐 수 있게)
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 컨트롤러 입력 막기 (내 캐릭터인 경우만 해당됨)
		if (Controller) DisableInput(Cast<APlayerController>(Controller));
	}
}

void ASFCharacter::ProcessRespawn(FVector NewLocation)
{
	CurrentState = ECharacterState::Idle;
	SetActorLocation(NewLocation);

	// UI 복구
	if (HPBarComponent)
	{
		USFHPBarWidget* HPWidget = Cast<USFHPBarWidget>(HPBarComponent->GetUserWidgetObject());
		if (HPWidget) HPWidget->UpdateHP(100.0f, 100.0f);
	}

	// 상태 복구
	StopAnimMontage();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	if (Controller) EnableInput(Cast<APlayerController>(Controller));
}

void ASFCharacter::SyncTransform(float DeltaTime)
{
	if (TargetLocation.IsZero()) return;

	// 1. 보간 (기존 동일)
	FVector CurrentLoc = GetActorLocation();
	FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLocation, DeltaTime, 15.0f);
	FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 15.0f);

	SetActorLocation(NewLoc);
	SetActorRotation(NewRot);

	// 2. 가짜 속도 계산
	FVector FakeVelocity = (NewLoc - CurrentLoc) / DeltaTime;
	GetCharacterMovement()->Velocity = FakeVelocity;

	float JumpThreshold = 100.0f; // 점프 감지 임계값

	// A. 속도가 빠르면 무조건 공중
	if (FMath::Abs(FakeVelocity.Z) > JumpThreshold)
	{
		if (GetCharacterMovement()->MovementMode != MOVE_Falling)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
	// B. 속도가 느릴 때 (땅 or 점프 정점) -> 바닥 체크
	else
	{
		FFindFloorResult FloorResult;

		GetCharacterMovement()->FindFloor(GetActorLocation(), FloorResult, false, nullptr);

		// 바닥이 있고(bBlockingHit) + 그 바닥이 걸을 수 있는 곳이라면(IsWalkableFloor)
		if (FloorResult.IsWalkableFloor())
		{
			if (GetCharacterMovement()->MovementMode != MOVE_Walking)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
		}
		else
		{
			// 바닥이 없거나, 너무 가파른 경사라면 -> 공중(Falling) 유지
			if (GetCharacterMovement()->MovementMode != MOVE_Falling)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			}
		}
	}
}

// 애니메이션 노티파이에서 호출할 콤보 체크 함수
void ASFCharacter::CheckNextCombo()
{
	if (bHasNextComboInput)
	{
		bHasNextComboInput = false; // 예약 초기화
		ComboIndex++; // 콤보 증가

		// Combo2, Combo3 섹션으로 점프!
		FString SectionName = FString::Printf(TEXT("Combo%d"), ComboIndex);
		PlayAnimMontage(AttackMontage, 1.0f, FName(*SectionName));

		USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
		if (GI)
		{
			PacketPlayerAttack AttackPacket;
			AttackPacket.Size = sizeof(PacketPlayerAttack);
			AttackPacket.Id = C_TO_S_PLAYER_ATTACK;
			AttackPacket.PlayerID = GI->MyPlayerID; // 내 ID

			GI->SendPacket(&AttackPacket, sizeof(AttackPacket));
			UE_LOG(LogTemp, Log, TEXT("[캐릭터] %d타 패킷 전송"), ComboIndex);
		}


	}
}