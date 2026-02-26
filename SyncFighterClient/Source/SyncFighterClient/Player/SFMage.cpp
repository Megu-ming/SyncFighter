#include "SFMage.h"
#include "Camera/CameraComponent.h" 
#include "Kismet/KismetMathLibrary.h"

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
	GetWorld()->SpawnActor<AActor>(BasicFireballClass, SpawnLoc, SpawnRot);

	UE_LOG(LogTemp, Log, TEXT("[마법사] %s 소켓에서 파이어볼 발사!"), *SocketName.ToString());
}