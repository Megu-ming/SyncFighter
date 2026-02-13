// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "MyNetworkActor.generated.h"

// 다른 플레이어의 정보를 담을 구조체
struct FRemotePlayerInfo
{
	class ASFCharacter* Character;       // 캐릭터 액터
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
	UPROPERTY(EditAnywhere, Category = "Network")
	TSubclassOf<class ASFCharacter> RemoteCharacterClass;

private:
	// 접속한 다른 플레이어들을 관리하는 목록 (ID : 액터)
	TMap<int32, FRemotePlayerInfo> RemotePlayers;
};
