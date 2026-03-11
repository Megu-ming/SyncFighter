#include "SFMage.h"
#include "Camera/CameraComponent.h" 
#include "Kismet/KismetMathLibrary.h"
#include "SyncFighterClient/SFGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void ASFMage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAimingQ && CurrentIndicator && FollowCamera)
	{
		FVector StartLoc = FollowCamera->GetComponentLocation();
		FVector ForwardVec = FollowCamera->GetForwardVector();
		FVector EndLoc = StartLoc + (ForwardVec * 5000.0f); // 50m 레이저

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this); // 나 자신 통과
		CollisionParams.AddIgnoredActor(CurrentIndicator); // 장판 자신도 통과

		// 바닥에 레이저가 닿았다면 장판 위치를 거기로 이동!
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, CollisionParams))
		{
			CurrentIndicator->SetActorLocation(HitResult.ImpactPoint);
		}
	}
}

void ASFMage::EndState()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	Super::EndState();
}

void ASFMage::ProcessBasicAttack()
{
	CancelAiming();

	Super::ProcessBasicAttack();
}

void ASFMage::ProcessSkillQ()
{
	if (!bIsAimingQ)
	{
		bIsAimingQ = true;

		if (SkillIndicatorClass)
		{
			CurrentIndicator = GetWorld()->SpawnActor<AActor>(SkillIndicatorClass, GetActorLocation(), FRotator::ZeroRotator);
		}

		UE_LOG(LogTemp, Warning, TEXT("[기데온] Q 스킬 조준 시작!"));
	}
	else
	{
		bIsAimingQ = false;

		if (CurrentIndicator)
		{
			FVector FinalTargetLoc = CurrentIndicator->GetActorLocation();
			CurrentIndicator->Destroy();
			CurrentIndicator = nullptr;

			if (!SkillQMagicClass) return;

			if (IsLocallyControlled() && FollowCamera)
			{
				FVector StartLoc = FollowCamera->GetComponentLocation();
				FVector ForwardVec = FollowCamera->GetForwardVector();
				FVector EndLoc = StartLoc + (ForwardVec * 5000.0f);

				FHitResult HitResult;
				FCollisionQueryParams CollisionParams;
				CollisionParams.AddIgnoredActor(this);

				if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, CollisionParams))
				{
					SkillQTargetLoc = HitResult.ImpactPoint;

					USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
					if (GI) GI->SendSkillPacket(0, SkillQTargetLoc);

					if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.0f);
				}
			}
		}
	}
	EndState();
}

void ASFMage::PlayRemoteSkillQ(FVector TargetLoc)
{
	SkillQTargetLoc = TargetLoc;
	if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.0f);
}

void ASFMage::ProcessSkillE()
{
	
}

void ASFMage::PlayRemoteSkillE(FVector TargetLoc)
{
	
}

void ASFMage::ProcessSkillR()
{
	CancelAiming();
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	SkillRTargetLoc = GetActorLocation();
	if (SkillRMontage) PlayAnimMontage(SkillRMontage, 1.0f);

	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI && IsLocallyControlled())
	{
		GI->SendSkillPacket(2, GetActorLocation());
	}
}

void ASFMage::PlayRemoteSkillR(FVector TargetLoc)
{
	SkillRTargetLoc = TargetLoc;
	if (SkillRMontage) PlayAnimMontage(SkillRMontage, 1.0f);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void ASFMage::SpawnSkillQMagic()
{
	if (SkillQMagicClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		// 기억해둔 좌표에 스폰!
		GetWorld()->SpawnActor<AActor>(SkillQMagicClass, SkillQTargetLoc, FRotator::ZeroRotator, SpawnParams);
	}
}

void ASFMage::SpawnSkillRMagic()
{
	if (SkillRMagicClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		// 기억해둔 좌표(기데온 본체)에 스폰!
		GetWorld()->SpawnActor<AActor>(SkillRMagicClass, SkillRTargetLoc, FRotator::ZeroRotator, SpawnParams);
	}
}

void ASFMage::FireMagic(FName SocketName)
{
	// 투사체 발사 함수
	if (!BasicFireballClass || !FollowCamera) return;

	FVector StartLoc = FollowCamera->GetComponentLocation();
	FVector ForwardVec = FollowCamera->GetForwardVector();
	FVector EndLoc = StartLoc + (ForwardVec * 10000.0f);

	FHitResult HitResult;
	FVector TargetPoint = EndLoc;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, CollisionParams))
	{
		TargetPoint = HitResult.ImpactPoint;
	}

	FVector SpawnLoc = GetMesh()->GetSocketLocation(SocketName);

	FRotator SpawnRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetPoint);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	GetWorld()->SpawnActor<AActor>(BasicFireballClass, SpawnLoc, SpawnRot, SpawnParams);

	if(IsLocallyControlled())
		UE_LOG(LogTemp, Log, TEXT("[마법사] %s 소켓에서 파이어볼 발사!"), *SocketName.ToString());
}

void ASFMage::CancelAiming()
{
	if (bIsAimingQ)
	{
		bIsAimingQ = false; // 1. 조준 상태 해제

		// 2. 바닥에 깔려있던 장판 파괴
		if (CurrentIndicator)
		{
			CurrentIndicator->Destroy();
			CurrentIndicator = nullptr;
		}

		UE_LOG(LogTemp, Warning, TEXT("[마법사] 다른 입력 감지: Q 스킬 조준 강제 취소!"));
	}
}
