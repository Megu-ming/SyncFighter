#include "SFMage.h"
#include "Camera/CameraComponent.h" 
#include "Kismet/KismetMathLibrary.h"
#include "SyncFighterClient/SFGameInstance.h"

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
	Super::EndState();
}

void ASFMage::ProcessBasicAttack()
{
	Super::ProcessBasicAttack();
}

void ASFMage::ProcessSkillQ()
{
	// ==========================================
	// 1. 첫 번째 Q 입력 (조준 시작)
	// ==========================================
	if (!bIsAimingQ)
	{
		bIsAimingQ = true;

		// 발밑에 장판 액터 소환
		if (SkillIndicatorClass)
		{
			CurrentIndicator = GetWorld()->SpawnActor<AActor>(SkillIndicatorClass, GetActorLocation(), FRotator::ZeroRotator);
		}

		UE_LOG(LogTemp, Warning, TEXT("[기데온] Q 스킬 조준 시작!"));
	}
	// ==========================================
	// 2. 두 번째 Q 입력 (스킬 진짜 발사!)
	// ==========================================
	else
	{
		bIsAimingQ = false; // 조준 끝!

		if (CurrentIndicator)
		{
			FVector FinalTargetLoc = CurrentIndicator->GetActorLocation();

			// 할 일 다 한 장판은 파괴
			CurrentIndicator->Destroy();
			CurrentIndicator = nullptr;

			if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.2f);
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
					FVector TargetGroundLoc = HitResult.ImpactPoint;

					// 1. 서버에 좌표 제보!
					USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
					if (GI) GI->SendSkillPacket(0, TargetGroundLoc);

					// 2. 내 화면에 이펙트 소환!
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = this;
					SpawnParams.Instigator = this;
					GetWorld()->SpawnActor<AActor>(SkillQMagicClass, TargetGroundLoc, FRotator::ZeroRotator, SpawnParams);
				}
			}
		}
	}
	EndState();
}

void ASFMage::PlayRemoteSkillQ(FVector TargetLoc)
{
	if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.2f); // 남의 캐릭터가 허우적거리는 모션 재생
	if (SkillQMagicClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		GetWorld()->SpawnActor<AActor>(SkillQMagicClass, TargetLoc, FRotator::ZeroRotator);
	}
}

void ASFMage::ProcessSkillE()
{
	if (SkillEMontage) PlayAnimMontage(SkillEMontage, 1.0f);

	// E 스킬 (중력장)은 투사체가 아니라 내 주변에 장판이 깔리는 것이므로, 
	// 나중에 파티클 시스템(VFX)을 직접 내 몸체에 Attach 하거나, 별도의 장판 액터를 발 밑에 스폰할 예정입니다.
	EndState();
	UE_LOG(LogTemp, Log, TEXT("[마법사] E 스킬 시전: 중력장 전개!"));
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