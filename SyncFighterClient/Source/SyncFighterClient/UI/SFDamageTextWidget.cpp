#include "SFDamageTextWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void USFDamageTextWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayAnimation(FloatingAnim);
	UKismetSystemLibrary::Delay(this, 0.5f, FLatentActionInfo());
	RemoveFromParent();
}

void USFDamageTextWidget::SetDamage(int32 Damage)
{
	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(Damage));
	}
}