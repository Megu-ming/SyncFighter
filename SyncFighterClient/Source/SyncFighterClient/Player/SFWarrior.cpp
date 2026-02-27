#include "SFWarrior.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SyncFighterClient/SFGameInstance.h"

void ASFWarrior::EndState()
{
	Super::EndState();
}

void ASFWarrior::ProcessBasicAttack()
{
	Super::ProcessBasicAttack();
}

void ASFWarrior::ProcessSkillQ()
{
	if (SkillQMontage) PlayAnimMontage(SkillQMontage, 1.2f);

	EndState();
	UE_LOG(LogTemp, Log, TEXT("[전사] Q 스킬 시전: 대지 강타!"));
}

void ASFWarrior::ProcessSkillE()
{
	if (SkillEMontage) PlayAnimMontage(SkillEMontage, 1.2f);

	EndState();
	UE_LOG(LogTemp, Log, TEXT("[전사] E 스킬 시전: 기절 타격!"));
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
