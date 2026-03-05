#include "SFPopupWidget.h"
#include "Components/Button.h"
#include "SyncFighterClient/SFGameInstance.h"

void USFPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (YesButton) YesButton->OnClicked.AddDynamic(this, &USFPopupWidget::OnYesClicked);
	if (NoButton) NoButton->OnClicked.AddDynamic(this, &USFPopupWidget::OnNoClicked);
}

void USFPopupWidget::OnYesClicked()
{
	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI) GI->RequestRegister(NewID, NewPW);
	SetVisibility(ESlateVisibility::Hidden);
}

void USFPopupWidget::OnNoClicked()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void USFPopupWidget::CacheInfo(FString ID, FString PW)
{
	NewID = ID;
	NewPW = PW;
}
