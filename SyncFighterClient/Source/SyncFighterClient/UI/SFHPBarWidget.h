#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h" // 프로그레스 바 사용
#include "SFHPBarWidget.generated.h"

UCLASS()
class SYNCFIGHTERCLIENT_API USFHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// HP 업데이트 함수 (0.0 ~ 1.0)
	void UpdateHP(float CurrentHP, float MaxHP);

protected:
	// ★ 핵심: 에디터에 있는 위젯과 C++ 변수를 자동 연결
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
};