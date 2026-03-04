#include "SFBlackhole.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SyncFighterClient/SFGameInstance.h"
#include "SyncFighterClient/Player/SFCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASFBlackhole::ASFBlackhole()
{
	PrimaryActorTick.bCanEverTick = true;

	// 파티클 컴포넌트 생성 및 루트로 지정
	BlackHoleParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BlackHoleParticle"));
	EndBlackHoleParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EndBlackHoleParticle"));
	RootComponent = BlackHoleParticle;
	EndBlackHoleParticle->SetupAttachment(GetRootComponent());
	EndBlackHoleParticle->bAutoActivate = false;
}

// Called when the game starts or when spawned
void ASFBlackhole::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(TickTimerHandle, this, &ASFBlackhole::ApplyDamageAndCC, 0.5f, true, 0.5f);
}

void ASFBlackhole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector StartLoc = GetActorLocation();

	for (auto& Pair : VictimBeams)
	{
		ASFCharacter* Victim = Pair.Key;
		UParticleSystemComponent* BeamComp = Pair.Value;

		if (IsValid(Victim) && IsValid(BeamComp))
		{
			// 빔의 시작점은 블랙홀 중앙, 끝점은 적의 명치(Z축 +50 정도)로 계속 업데이트!
			BeamComp->SetBeamSourcePoint(0, StartLoc, 0);
			BeamComp->SetBeamTargetPoint(0, Victim->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f), 0);
		}
	}
}

void ASFBlackhole::ApplyDamageAndCC()
{
	TickCount++;
	FVector DamageLoc = GetActorLocation(); // 이제 블랙홀 액터 자신의 위치가 중심!
	float DamageRadius = 600.0f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetInstigator()); // 나를 소환한 기데온(주인)은 데미지를 안 받게 면역 처리!

	TArray<AActor*> OutActors;
	ASFCharacter* InstigatorChar = Cast<ASFCharacter>(GetInstigator());
	bool bIsMyBlackHole = (InstigatorChar && InstigatorChar->IsLocallyControlled());

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), DamageLoc, DamageRadius, ObjectTypes, nullptr, ActorsToIgnore, OutActors
	);

	if (bHit)
	{
		// 네트워크 제보를 위해 주인의 GameInstance를 가져옴
		USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
		if (GI)
		{
			for (AActor* HitActor : OutActors)
			{
				ASFCharacter* HitChar = Cast<ASFCharacter>(HitActor);
				if (HitChar && HitChar != GetInstigator())
				{
					if (bIsMyBlackHole)
					{
						GI->SendHitReq(HitChar->PlayerID, 10);
					}

					// 희생자 명단에 없으면 늪(CC기)에 빠뜨림
					if (!Victims.Contains(HitChar))
					{
						HitChar->SetBlackHoleState(true, DamageLoc);
						Victims.Add(HitChar);

						if (BeamParticleClass)
						{
							UParticleSystemComponent* BeamComp = UGameplayStatics::SpawnEmitterAttached(BeamParticleClass, RootComponent);
							if (BeamComp)
							{
								VictimBeams.Add(HitChar, BeamComp); // 명부에 "희생자-빔" 짝지어서 기록
							}
						}
					}
				}
			}
		}
	}

	// 6번(3초) 때렸으면 자기 자신을 파괴!
	if (TickCount >= 6)
	{
		EndBlackhole();
	}
}

void ASFBlackhole::EndBlackhole()
{
	// 타이머 정리
	GetWorldTimerManager().ClearTimer(TickTimerHandle);

	// 내가 물고 있던 모든 희생자들의 CC기를 확실하게 풀어줌!
	for (ASFCharacter* Victim : Victims)
	{
		if (IsValid(Victim))
		{
			Victim->SetBlackHoleState(false, FVector::ZeroVector);
		}
	}
	Victims.Empty();

	for (auto& Pair : VictimBeams)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Deactivate(); // 빔 끄기
		}
	}
	VictimBeams.Empty();

	if (BlackHoleParticle) BlackHoleParticle->Deactivate();
	if (EndBlackHoleParticle) EndBlackHoleParticle->Activate();

	SetLifeSpan(2.0f);
	UE_LOG(LogTemp, Warning, TEXT("[블랙홀] 액터 소멸, 모든 CC기 해제 완료"));
}
