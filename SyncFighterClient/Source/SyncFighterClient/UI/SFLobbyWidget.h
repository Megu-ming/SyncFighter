// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFLobbyWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginSuccessDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterChangedDelegate, int32, NewClassType);

class UWidgetSwitcher;
class UEditableTextBox;
class UButton;
class UTextBlock;
class UWidgetAnimation;
class USFPopupWidget;

UCLASS()
class SYNCFIGHTERCLIENT_API USFLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MainSwitcher;

	// --- 로그인 창 ---
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* IDInput;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PWInput;

	UPROPERTY(meta = (BindWidget))
	UButton* LoginBtn;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* Visible;

	// --- 캐릭터 선택 창 (스크린샷에 맞게 수정) ---
	UPROPERTY(meta = (BindWidget))
	UButton* LeftBtn;  // << 버튼

	UPROPERTY(meta = (BindWidget))
	UButton* RightBtn; // >> 버튼

	UPROPERTY(meta = (BindWidget))
	UButton* StartGameBtn; // 게임 시작 버튼

	// --- 상태 변수 ---
	UPROPERTY(meta = (BindWidget))
	USFPopupWidget* PopupWidget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Select")
	TSubclassOf<AActor> WarriorActorClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Select")
	TSubclassOf<AActor> MageActorClass;

	UPROPERTY()
	AActor* RefWarrior;

	UPROPERTY()
	AActor* RefMage;

	int32 SelectedClassType = 0;	// 0 : 전사 1 : 마법사

	UFUNCTION()
	void OnLoginClicked();

	UFUNCTION()
	void OnLeftClicked();

	UFUNCTION()
	void OnRightClicked();

	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnLoginResultReceived(int32 ResultCode);

	void UpdateCharacterVisibility();

	UPROPERTY(BlueprintAssignable, Category = "Lobby Event")
	FOnCharacterChangedDelegate OnCharacterChangedEvent;
};
