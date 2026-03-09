// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "MyNetworkActor.generated.h"

class USFHUDWidget;
class USFResultWidget;

// 다른 플레이어의 정보를 담을 구조체
struct FRemotePlayerInfo
{
	class ASFCharacter* Character;      // 캐릭터 액터
	class ASFPlayerState* PlayerState;	// 원격 유저의 기록부

	FVector TargetPos;   // 서버에서 받은 목표 위치
	FRotator TargetRot;  // 서버에서 받은 목표 회전
	bool bIsDead = false;
};

UCLASS()
class SYNCFIGHTERCLIENT_API AMyNetworkActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyNetworkActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// 블루프린트에서 만든 위젯 디자인을 할당할 칸
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USFHUDWidget> HUDWidgetClass;

	// 생성된 위젯을 들고 있을 포인터
	UPROPERTY()
	USFHUDWidget* HUDWidget;

public:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<USFResultWidget> ResultWidgetClass;

	UPROPERTY()
	USFResultWidget* ResultWidget;

public:
	UPROPERTY(EditAnywhere, Category = "Network|Class")
	TSubclassOf<class ASFCharacter> WarriorClass;

	UPROPERTY(EditAnywhere, Category = "Network|Class")
	TSubclassOf<class ASFCharacter> MageClass;

private:
	// 접속한 다른 플레이어들을 관리하는 목록 (ID : 액터)
	TMap<int32, FRemotePlayerInfo> RemotePlayers;
};
