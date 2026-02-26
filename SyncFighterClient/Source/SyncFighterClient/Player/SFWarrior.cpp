#include "SFWarrior.h"

void ASFWarrior::EndState()
{
	Super::EndState();
}

void ASFWarrior::ProcessBasicAttack()
{
	Super::ProcessBasicAttack();

	if (CurrentState == ECharacterState::BasicAttacking)
	{
		// 공격 도중 클릭 시 -> 다음 콤보 예약
		if (ComboIndex < 4)
		{
			bHasNextComboInput = true;
			UE_LOG(LogTemp, Log, TEXT("[전사] 다음 콤보 예약됨!"));
		}
	}
}

void ASFWarrior::ProcessSkillQ()
{
	if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.2f);

	UE_LOG(LogTemp, Log, TEXT("[전사] Q 스킬 시전: 대지 강타!"));
}

void ASFWarrior::ProcessSkillE()
{
	if (SkillEMontage) PlayAnimMontage(SkillEMontage, 1.2f);

	UE_LOG(LogTemp, Log, TEXT("[전사] E 스킬 시전: 기절 타격!"));
}
