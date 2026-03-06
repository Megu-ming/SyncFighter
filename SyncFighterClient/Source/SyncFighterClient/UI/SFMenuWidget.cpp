#include "SFMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void USFMenuWidget::NativeConstruct()
{
	if (ResumeBtn) ResumeBtn->OnClicked.AddDynamic(this, &USFMenuWidget::OnResumeClicked);
	if (QuitBtn) QuitBtn->OnClicked.AddDynamic(this, &USFMenuWidget::OnQuitClicked);
}

void USFMenuWidget::OnResumeClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		SetVisibility(ESlateVisibility::Hidden);
	}
}

void USFMenuWidget::OnQuitClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}
