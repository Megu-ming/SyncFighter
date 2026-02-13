#include "SFCharacter.h"
#include "Components/WidgetComponent.h"
#include "SyncFighterClient/UI/SFHPBarWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ASFCharacter::ASFCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. 위젯 컴포넌트 설정 (이사 옴)
	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComponent->SetupAttachment(GetRootComponent());
	HPBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	HPBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarComponent->SetDrawSize(FVector2D(150.0f, 20.0f));
}

void ASFCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASFCharacter::ProcessAttack()
{
	if (bIsDead) return;
	if (AttackMontage) PlayAnimMontage(AttackMontage);
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
	if (RemainingHP <= 0 && !bIsDead)
	{
		bIsDead = true;
		if (DeathMontage) PlayAnimMontage(DeathMontage);

		// 캡슐 충돌 끄기 (시체 위로 걸어다닐 수 있게)
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 컨트롤러 입력 막기 (내 캐릭터인 경우만 해당됨)
		if (Controller) DisableInput(Cast<APlayerController>(Controller));
	}
}

void ASFCharacter::ProcessRespawn(FVector NewLocation)
{
	bIsDead = false;
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