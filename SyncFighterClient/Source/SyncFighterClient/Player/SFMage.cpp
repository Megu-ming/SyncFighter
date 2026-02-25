#include "SFMage.h"

void ASFMage::EndState()
{
	Super::EndState();
}

void ASFMage::ProcessBasicAttack()
{
	Super::ProcessBasicAttack();

	if (CurrentState == ECharacterState::Attacking)
	{
		// 공격 도중 클릭 시 -> 다음 콤보 예약
		if (ComboIndex < 3)
		{
			bHasNextComboInput = true;
			UE_LOG(LogTemp, Log, TEXT("[마법사] 다음 콤보 예약됨!"));
		}
	}

	if (BasicFireballClass)
	{
		FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 80.0f;
		FRotator SpawnRot = GetActorRotation();
		GetWorld()->SpawnActor<AActor>(BasicFireballClass, SpawnLoc, SpawnRot);
	}
	UE_LOG(LogTemp, Log, TEXT("[마법사] 파이어볼 발사!"));
}

void ASFMage::ProcessSkillQ()
{
	if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.2f);

	if (SkillQMagicClass)
	{
		FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.0f;
		FRotator SpawnRot = GetActorRotation();
		GetWorld()->SpawnActor<AActor>(SkillQMagicClass, SpawnLoc, SpawnRot);
	}
	UE_LOG(LogTemp, Log, TEXT("[마법사] Q 스킬 시전: 특수 마법 발동!"));
}

void ASFMage::ProcessSkillE()
{
	if (SkillEMontage) PlayAnimMontage(SkillEMontage, 1.0f);

	// E 스킬 (중력장)은 투사체가 아니라 내 주변에 장판이 깔리는 것이므로, 
	// 나중에 파티클 시스템(VFX)을 직접 내 몸체에 Attach 하거나, 별도의 장판 액터를 발 밑에 스폰할 예정입니다.
	UE_LOG(LogTemp, Log, TEXT("[마법사] E 스킬 시전: 중력장 전개!"));
}
