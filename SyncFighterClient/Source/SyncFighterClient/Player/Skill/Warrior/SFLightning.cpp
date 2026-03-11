#include "SFLightning.h"
#include "Components/SphereComponent.h"
#include "SyncFighterClient/SFGameInstance.h"
#include "SyncFighterClient/Player/SFCharacter.h"

ASFLightning::ASFLightning()
{
	PrimaryActorTick.bCanEverTick = false;
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetSphereRadius(350.0f);

	SphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ASFLightning::Strike);
}

void ASFLightning::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(2.0f);
}

void ASFLightning::Strike(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (DamagedActors.Contains(OtherActor)) return;

	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (!GI) return;

	ASFCharacter* HitChar = Cast<ASFCharacter>(OtherActor);

	if (HitChar && HitChar != GetInstigator() && HitChar->CurrentState != ECharacterState::Dead)
	{
		DamagedActors.Add(OtherActor);

		GI->SendHitReq(HitChar->PlayerID, 40);
		UE_LOG(LogTemp, Warning, TEXT("⚡ 벼락 적중! 대상 ID: %d"), HitChar->PlayerID);
	}
}