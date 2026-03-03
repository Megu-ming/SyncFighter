#include "SFWarrior.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SyncFighterClient/SFGameInstance.h"
#include "Camera/CameraComponent.h"

void ASFWarrior::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 조준 중이고 장판이 띄워져 있다면?
	if (bIsAimingQ && CurrentIndicator && FollowCamera)
	{
		FVector StartLoc = FollowCamera->GetComponentLocation();
		FVector ForwardVec = FollowCamera->GetForwardVector();
		FVector EndLoc = StartLoc + (ForwardVec * 5000.0f); // 50m 레이저

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		CollisionParams.AddIgnoredActor(CurrentIndicator);

		// 바닥 좌표를 찾아 장판 이동
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, CollisionParams))
		{
			CurrentIndicator->SetActorLocation(HitResult.ImpactPoint);
		}
	}
}

void ASFWarrior::EndState()
{
	Super::EndState();
}

void ASFWarrior::ProcessBasicAttack()
{
	CancelAiming();
	if (bIsWeaponThrown) RecallWeapon();
	if (bIsRecalled)
	{
		USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
		if (GI && IsLocallyControlled())
		{
			// 패킷 구조체 수동 전송
			PacketPlayerAttack AttackPkt;
			AttackPkt.Size = sizeof(PacketPlayerAttack);
			AttackPkt.Id = C_TO_S_PLAYER_ATTACK;
			AttackPkt.PlayerID = GI->MyPlayerID;

			GI->SendPacket(&AttackPkt, sizeof(AttackPkt));
		}
		return;
	}

	Super::ProcessBasicAttack();
}

void ASFWarrior::ProcessSkillQ()
{
	if (bIsWeaponThrown) RecallWeapon();
	if (bIsRecalled)
	{
		USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
		if (GI && IsLocallyControlled())
		{
			PacketPlayerAttack AttackPkt;
			AttackPkt.Size = sizeof(PacketPlayerAttack);
			AttackPkt.Id = C_TO_S_PLAYER_ATTACK;
			AttackPkt.PlayerID = GI->MyPlayerID;

			GI->SendPacket(&AttackPkt, sizeof(AttackPkt));
		}
		return;
	}

	if (!bIsAimingQ)
	{
		bIsAimingQ = true;
		if (SkillIndicatorClass)
		{
			CurrentIndicator = GetWorld()->SpawnActor<AActor>(SkillIndicatorClass, GetActorLocation(), FRotator::ZeroRotator);
		}
		UE_LOG(LogTemp, Warning, TEXT("[전사] Q 스킬 조준 시작!"));
	}
	else
	{
		bIsAimingQ = false;

		if (CurrentIndicator)
		{
			SkillQTargetLoc = CurrentIndicator->GetActorLocation();

			CurrentIndicator->Destroy();
			CurrentIndicator = nullptr;

			HitActors.Empty();
			HitActors.Add(this);

			CurrentState = ECharacterState::SkillAttacking;

			// 칼던지는 몽타주 실행
			if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.0f);

			// 서버로 스킬 사용 제보
			USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
			if (GI && IsLocallyControlled())
			{
				GI->SendSkillPacket(0, SkillQTargetLoc);
				UE_LOG(LogTemp, Warning, TEXT("[전사] 쾅 Q스킬 시전!"));
			}
		}
	}
}

void ASFWarrior::ProcessSkillE()
{
	// 1. 조준 중이었다면 취소
	CancelAiming();

	CurrentState = ECharacterState::SkillAttacking;
	if (bIsWeaponThrown)
	{
		// 1. 칼이 바닥에 있을 때: 기 모으기(Intro)만 재생하고 기다립니다.
		if (SkillEIntroMontage)
		{
			PlayAnimMontage(SkillEIntroMontage, 1.0f);
		}
	}
	else
	{
		// 2. 칼이 내 손에 있을 때: 텔레포트 없이 제자리에서 바로 크게 휘두릅니다.
		if (SkillEMontage)
		{
			PlayAnimMontage(SkillEMontage, 1.0f);
		}
	}

	// 3. 패킷은 E를 누른 즉시 무조건 보냅니다! (서버도 똑같이 애니메이션을 틀어야 하니까요)
	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI && IsLocallyControlled())
	{
		GI->SendSkillPacket(1, GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("[전사] E스킬 시전 패킷 전송!"));
	}
}

void ASFWarrior::PlayRemoteSkillQ(FVector TargetLoc)
{
	SkillQTargetLoc = TargetLoc;
	if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.0f);
}

void ASFWarrior::ApplySkillQDamage()
{
	bIsWeaponThrown = true;

	if (SkillQMagicClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		GroundedSword = GetWorld()->SpawnActor<AActor>(SkillQMagicClass, SkillQTargetLoc, FRotator::ZeroRotator, SpawnParams);
	}

	if (!IsLocallyControlled()) return;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> OutActors; // 맞은 애들이 담길 임시 명단

	// ★ 핵심: ActorsToIgnore 대신, 이미 나 자신이 포함된 HitActors 명부를 그대로 넘겨줍니다!
	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), SkillQTargetLoc, 400.0f, ObjectTypes, nullptr, HitActors, OutActors
	);

	if (bHit)
	{
		USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
		if (GI)
		{
			// 명단에 있는 모든 적의 체력을 깎으라고 서버에 제보!
			for (AActor* HitActor : OutActors)
			{
				ASFCharacter* HitChar = Cast<ASFCharacter>(HitActor);
				if (HitChar && !HitActors.Contains(HitChar))
				{
					HitActors.Add(HitChar);

					GI->SendHitReq(HitChar->PlayerID, 50); // 번개 강타 데미지 (50)
					UE_LOG(LogTemp, Warning, TEXT("[전사] 번개 강타 적중! 대상 ID: %d"), HitChar->PlayerID);
				}
			}
		}
	}
}

void ASFWarrior::BeginMeleeAttack()
{
	HitActors.Empty();
	HitActors.Add(this);
}

void ASFWarrior::CheckMeleeHit()
{
	if (!IsLocallyControlled()) return;

	FVector StartLoc = GetMesh()->GetSocketLocation(FName("FX_weapon_base"));
	FVector EndLoc = GetMesh()->GetSocketLocation(FName("FX_weapon_tip"));

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	FHitResult HitResult;

	bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		GetWorld(),
		StartLoc,
		EndLoc,
		30.0f,
		ObjectTypes, 
		false,
		HitActors,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red, FLinearColor::Green, 2.0f
	);

	if (bHit && HitResult.GetActor())
	{
		ASFCharacter* HitChar = Cast<ASFCharacter>(HitResult.GetActor());

		// 블랙리스트 체크
		if (HitChar && !HitActors.Contains(HitChar))
		{
			// 1. 타격 성공! 다음 프레임부터는 안 맞도록 명부에 올림
			HitActors.Add(HitChar);

			// 2. 서버로 데미지 제보
			USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
			if (GI)
			{
				GI->SendHitReq(HitChar->PlayerID, 15);
				UE_LOG(LogTemp, Warning, TEXT("[전사] 궤적 타격 적중! 대상 ID: %d"), HitChar->PlayerID);
			}
		}
	}
}

/// <summary>
/// 조준(스킬 시전 전) 동작 해제 함수
/// </summary>
void ASFWarrior::CancelAiming()
{
	if (bIsAimingQ)
	{
		bIsAimingQ = false;

		if (CurrentIndicator)
		{
			CurrentIndicator->Destroy();
			CurrentIndicator = nullptr;
		}

		UE_LOG(LogTemp, Warning, TEXT("[전사] 다른 입력 감지: 조준 강제 취소!"));
	}
}

void ASFWarrior::RecallWeapon()
{
	if (bIsWeaponThrown && !bIsRecalled)
	{
		bIsRecalled = true;
		
		if (RecallWeaponMontage) PlayAnimMontage(RecallWeaponMontage);
		if (GroundedSword)
		{
			GroundedSword->Destroy();
			GroundedSword = nullptr;
		}

		bIsWeaponThrown = false;
		UE_LOG(LogTemp, Warning, TEXT("[전사] 대검 회수 완료!"));
	}
}

void ASFWarrior::EndRecalled()
{
	bIsRecalled = false;
}

void ASFWarrior::Teleport()
{
	FVector DestLoc = SkillQTargetLoc;
	DestLoc.Z += 90.0f;
	SetActorLocation(DestLoc);

	bIsWeaponThrown = false;
	if (IsValid(GroundedSword))
	{
		GroundedSword->Destroy();
		GroundedSword = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("[전사] 텔레포트 완료! 휘두르기 공격 연계 시작!"));

	// 3. 순간이동이 끝났으니, 이제 크게 휘두르는 진짜 공격 애니메이션으로 자연스럽게 바통 터치!
	if (SkillEMontage)
	{
		PlayAnimMontage(SkillEMontage, 1.0f);
	}
}