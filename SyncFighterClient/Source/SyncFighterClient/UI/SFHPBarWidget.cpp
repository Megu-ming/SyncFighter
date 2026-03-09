#include "SFHPBarWidget.h"

void USFHPBarWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HealthBar)
	{
		if (MaxHP > 0)
		{
			float Percent = CurrentHP / MaxHP;
			HealthBar->SetPercent(Percent);
		}
	}
}