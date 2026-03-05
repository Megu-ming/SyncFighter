#include "SFLobbyWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "SyncFighterClient/SFGameInstance.h"
#include "SFPopupWidget.h"

void USFLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LoginBtn) LoginBtn->OnClicked.AddDynamic(this, &USFLobbyWidget::OnLoginClicked);
	if (LeftBtn) LeftBtn->OnClicked.AddDynamic(this, &USFLobbyWidget::OnLeftClicked);
	if (RightBtn) RightBtn->OnClicked.AddDynamic(this, &USFLobbyWidget::OnRightClicked);
	if (StartGameBtn) StartGameBtn->OnClicked.AddDynamic(this, &USFLobbyWidget::OnStartGameClicked);

	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI) GI->OnLoginResult.AddDynamic(this, &USFLobbyWidget::OnLoginResultReceived);

	if (MainSwitcher) MainSwitcher->SetActiveWidgetIndex(0);

	if (WarriorActorClass)
	{
		RefWarrior = UGameplayStatics::GetActorOfClass(GetWorld(), WarriorActorClass);
	}
	if (MageActorClass)
	{
		RefMage = UGameplayStatics::GetActorOfClass(GetWorld(), MageActorClass);
	}
	if (RefWarrior) RefWarrior->SetActorHiddenInGame(true);
	if (RefMage) RefMage->SetActorHiddenInGame(true);

	PopupWidget->SetVisibility(ESlateVisibility::Hidden);
}

void USFLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI) GI->CheckLoginPackets();
}

void USFLobbyWidget::OnLoginClicked()
{
	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI && IDInput && PWInput)
	{
		GI->RequestLogin(IDInput->GetText().ToString(), PWInput->GetText().ToString());
	}
}

void USFLobbyWidget::OnLeftClicked()
{
	SelectedClassType = (SelectedClassType == 0) ? 1 : 0;
	UpdateCharacterVisibility();
}

void USFLobbyWidget::OnRightClicked()
{
	SelectedClassType = (SelectedClassType == 0) ? 1 : 0;
	UpdateCharacterVisibility();
}

void USFLobbyWidget::OnStartGameClicked()
{
	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->MyClassType = SelectedClassType;
		UGameplayStatics::OpenLevel(GetWorld(), FName("ThirdPersonMap"));
	}
}

void USFLobbyWidget::OnLoginResultReceived(int32 ResultCode)
{
	// 로그인 성공, 회원가입 성공
	if (ResultCode == 0 || ResultCode == 3) // 로그인 성공
	{
		if (MainSwitcher) MainSwitcher->SetActiveWidgetIndex(1); // 1: 캐릭터 선택 화면으로 전환

		SelectedClassType = 0;
		UpdateCharacterVisibility();
	}
	// 패스워드만 틀렸을 때
	else if (ResultCode == 1) 
	{
		PlayAnimation(Visible);
	}
	// 없는 아이디 입력했을 때
	else if(ResultCode == 2)
	{
		PopupWidget->SetVisibility(ESlateVisibility::Visible);
		PopupWidget->CacheInfo(IDInput->GetText().ToString(), PWInput->GetText().ToString());
	}
	// 회원가입 직전에 같은 아이디 때문에 회원가입 실패
	else if (ResultCode == 4)
	{
		// 경고창 띄워주면서 이미 가입된 아이디 입니다 출력 ㄱ
	}
}

void USFLobbyWidget::UpdateCharacterVisibility()
{
	if (SelectedClassType == 0) // 전사 선택
	{
		if (RefWarrior) RefWarrior->SetActorHiddenInGame(false); // 보이기
		if (RefMage) RefMage->SetActorHiddenInGame(true);        // 숨기기
	}
	else // 마법사 선택
	{
		if (RefWarrior) RefWarrior->SetActorHiddenInGame(true);  // 숨기기
		if (RefMage) RefMage->SetActorHiddenInGame(false);       // 보이기
	}
}
