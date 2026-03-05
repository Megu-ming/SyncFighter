#include "SFHUDWidget.h"
#include "Components/TextBlock.h"

void USFHUDWidget::UpdateTimer(int32 TimeInSeconds)
{
	if (TimerText)
	{
		// 180초 -> "03 : 00" 형태로 변환
		int32 Minutes = TimeInSeconds / 60;
		int32 Seconds = TimeInSeconds % 60;

		FString TimeString = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
		TimerText->SetText(FText::FromString(TimeString));
	}
}

void USFHUDWidget::UpdateMyScore(int32 Kills, int32 Deaths)
{
	if (MyScoreText)
	{
		FString ScoreStr = FString::Printf(TEXT("MY SCORE\nK: %d / D: %d"), Kills, Deaths);
		MyScoreText->SetText(FText::FromString(ScoreStr));
	}
}

void USFHUDWidget::UpdateEnemyScore(int32 Kills, int32 Deaths)
{
	if (EnemyScoreText)
	{
		FString ScoreStr = FString::Printf(TEXT("ENEMY\nK: %d / D: %d"), Kills, Deaths);
		EnemyScoreText->SetText(FText::FromString(ScoreStr));
	}
}
