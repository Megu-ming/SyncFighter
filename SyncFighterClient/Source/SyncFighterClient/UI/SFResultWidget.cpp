#include "SFResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USFResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReturnButton)
	{
		ReturnButton->OnClicked.AddDynamic(this, &USFResultWidget::OnReturnButtonClicked);
	}
}

void USFResultWidget::SetResultInfo(bool bIsWinner, int32 MyKills, int32 MyDeaths)
{
	if (ResultText)
	{
		FString ResultStr = bIsWinner ? TEXT("VICTORY!") : TEXT("DEFEAT...");
		ResultText->SetText(FText::FromString(ResultStr));

		// 승패에 따라 색상 변경 (승리: 파랑, 패배: 빨강 등)
		ResultText->SetColorAndOpacity(bIsWinner ? FSlateColor(FLinearColor::Blue) : FSlateColor(FLinearColor::Red));
	}

	if (FinalScoreText)
	{
		FString ScoreStr = FString::Printf(TEXT("최종 성적\nKills: %d  |  Deaths: %d"), MyKills, MyDeaths);
		FinalScoreText->SetText(FText::FromString(ScoreStr));
	}
}

void USFResultWidget::OnReturnButtonClicked()
{
	RemoveFromParent();

	// 2. 조작 모드 원래대로 복구 (마우스 숨기기 등)
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	// (임시) 전투맵 리로드
	FTimerHandle LevelLoadTimer;
	GetWorld()->GetTimerManager().SetTimer(LevelLoadTimer, [this]()
		{
			FString CleanLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
			UGameplayStatics::OpenLevel(GetWorld(), FName(*CleanLevelName));
		}, 0.1f, false);
}
