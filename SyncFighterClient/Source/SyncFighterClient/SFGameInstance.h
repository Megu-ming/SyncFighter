#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "SyncFighterStructs.h" // 아까 만든 프로토콜 헤더
#include "SFGameInstance.generated.h"

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
	FSocket* Socket;
	FString ServerIP = "127.0.0.1";
	int32 ServerPort = 7777;
	int32 MyPlayerID = -1;
};