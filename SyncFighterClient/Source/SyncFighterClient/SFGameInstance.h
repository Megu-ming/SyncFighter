#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "SyncFighterStructs.h" // 아까 만든 프로토콜 헤더
#include "SFGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginResultDelegate, int32, ResultCode);

UCLASS()
class SYNCFIGHTERCLIENT_API USFGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	// 서버 연결 함수
	void ConnectToGameServer();

	// 데이터 보내기 (어디서든 호출 가능하게)
	void SendPacket(void* Packet, int32 Size);

	// 데이터 받기 (소켓에서 꺼내서 큐에 담아두기)
	// 나중에 Actor가 Tick에서 이 함수를 통해 데이터를 가져갑니다.
	bool HandleRecv(TArray<uint8>& OutData);

public:
	// UI에서 호출할 로그인 요청 함수
	UFUNCTION(BlueprintCallable, Category = "Network|Login")
	void RequestLogin(FString UserID, FString Password);

	// UI에서 호출할 회원가입 요청 함수
	UFUNCTION(BlueprintCallable, Category = "Network|Login")
	void RequestRegister(FString UserID, FString Password);

	// 1. UI에서 바인딩할 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Network|Login")
	FOnLoginResultDelegate OnLoginResult;

	// 2. 로비에서 패킷을 계속 확인할 함수
	UFUNCTION(BlueprintCallable, Category = "Network|Login")
	void CheckLoginPackets();

public:
	FSocket* Socket;
	FString ServerIP = "127.0.0.1";
	int32 ServerPort = 7777;
	int32 MyPlayerID = -1;
};