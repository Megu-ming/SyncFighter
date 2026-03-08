#include "SFDamageTextActor.h"
#include "Components/WidgetComponent.h"
#include "SFDamageTextWidget.h"

// Sets default values
ASFDamageTextActor::ASFDamageTextActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DamageWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidgetComp"));
	RootComponent = DamageWidgetComp;

	DamageWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComp->SetDrawSize(FVector2D(200.0f, 50.0f));
}

// Called when the game starts or when spawned
void ASFDamageTextActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetLifeSpan(1.0f);
}

void ASFDamageTextActor::InitializeDamage(int32 DamageAmount)
{
	if (DamageWidgetComp)
	{
		// 위젯 컴포넌트 안의 실제 UI 객체를 가져와서 데미지 값 전달
		USFDamageTextWidget* DamageUI = Cast<USFDamageTextWidget>(DamageWidgetComp->GetUserWidgetObject());
		if (DamageUI)
		{
			DamageUI->SetDamage(DamageAmount);
		}
	}
}