// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyncFighterClientGameMode.h"
#include "SyncFighterClientCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ASyncFighterClientGameMode::ASyncFighterClientGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ASyncFighterClientGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC)
	{
		// 2. 입력 모드를 '게임 전용(Game Only)'으로 설정
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		// 3. UI 조작용으로 켜뒀던 마우스 커서 다시 숨기기
		PC->bShowMouseCursor = false;

		UE_LOG(LogTemp, Warning, TEXT("GameMode: Input mode set to Game Only."));
	}
}