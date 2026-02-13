#include "SFHPBarWidget.h"

void USFHPBarWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HealthBar)
	{
		if (MaxHP > 0)
		{
			// 퍼센트 계산 (0.0 ~ 1.0)
			float Percent = CurrentHP / MaxHP;
			HealthBar->SetPercent(Percent);
		}
	}
}