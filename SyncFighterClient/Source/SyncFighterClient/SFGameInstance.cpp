#include "SFGameInstance.h"

void USFGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Warning, TEXT("GameInstance Init!"));
	ConnectToGameServer();
}

void USFGameInstance::Shutdown()
{
	Super::Shutdown();
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
	}
}

void USFGameInstance::ConnectToGameServer()
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("DefaultSocket"), false);

	FIPv4Address IP;
	FIPv4Address::Parse(ServerIP, IP);

	TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	Addr->SetIp(IP.Value);
	Addr->SetPort(ServerPort);

	UE_LOG(LogTemp, Warning, TEXT("서버에 접속 시도 중..."));
	bool bConnected = Socket->Connect(*Addr);

	if (bConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버 접속 성공!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("서버 접속 실패!"));
	}
}

void USFGameInstance::SendPacket(void* Packet, int32 Size)
{
	if (Socket && Socket->GetConnectionState() == SCS_Connected)
	{
		int32 BytesSent = 0;
		Socket->Send((uint8*)Packet, Size, BytesSent);
	}
}

// 소켓에 데이터가 있으면 읽어서 OutData에 담아주는 함수
bool USFGameInstance::HandleRecv(TArray<uint8>& OutData)
{
	if (Socket == nullptr || Socket->GetConnectionState() != SCS_Connected) return false;

	uint32 PendingDataSize = 0;
	if (Socket->HasPendingData(PendingDataSize))
	{
		OutData.SetNumUninitialized(PendingDataSize);
		int32 BytesRead = 0;
		Socket->Recv(OutData.GetData(), PendingDataSize, BytesRead);
		return true;
	}
	return false;
}

void USFGameInstance::RequestLogin(FString UserID, FString Password)
{
	PacketLoginReq Pkt;
	Pkt.Size = sizeof(PacketLoginReq);
	Pkt.Id = C_TO_S_LOGIN_REQ;

	// FString을 UTF8 char 배열로 안전하게 복사
	strncpy_s(Pkt.UserID, TCHAR_TO_UTF8(*UserID), sizeof(Pkt.UserID) - 1);
	strncpy_s(Pkt.Password, TCHAR_TO_UTF8(*Password), sizeof(Pkt.Password) - 1);

	SendPacket(&Pkt, sizeof(Pkt));

	UE_LOG(LogTemp, Log, TEXT("Login Request Sent: ID=%s, Class=%d"), *UserID);
}

void USFGameInstance::RequestRegister(FString UserID, FString Password)
{
	PacketRegisterReq Pkt;
	Pkt.Size = sizeof(PacketRegisterReq);
	Pkt.Id = C_TO_S_REGISTER_REQ;

	strncpy_s(Pkt.UserID, TCHAR_TO_UTF8(*UserID), sizeof(Pkt.UserID) - 1);
	strncpy_s(Pkt.Password, TCHAR_TO_UTF8(*Password), sizeof(Pkt.Password) - 1);

	SendPacket(&Pkt, sizeof(Pkt));

	UE_LOG(LogTemp, Log, TEXT("Register Request Sent: ID=%s"), *UserID);
}

void USFGameInstance::RequestEnterGame(int32 ClassType)
{
	MyClassType = ClassType; // 금고에 저장

	PacketEnterGameReq Pkt;
	Pkt.Size = sizeof(PacketEnterGameReq);
	Pkt.Id = C_TO_S_ENTER_GAME_REQ;
	Pkt.ClassType = ClassType;

	SendPacket(&Pkt, sizeof(Pkt));
}

void USFGameInstance::CheckLoginPackets()
{
	TArray<uint8> RecvBuffer;
	if (HandleRecv(RecvBuffer))
	{
		uint8* Buffer = RecvBuffer.GetData();
		int32 BytesRead = RecvBuffer.Num();
		int32 ProcessedBytes = 0;

		while (ProcessedBytes + sizeof(PacketHeader) <= (uint32)BytesRead)
		{
			PacketHeader* Header = (PacketHeader*)(Buffer + ProcessedBytes);
			if (ProcessedBytes + Header->Size > BytesRead) break;

			// 로그인 응답 패킷(11) 처리
			if (Header->Id == S_TO_C_LOGIN_RES)
			{
				PacketLoginRes* ResPkt = (PacketLoginRes*)(Buffer + ProcessedBytes);

				// 성공 시 내 플레이어 ID 저장
				if (ResPkt->ResultCode == 0 || ResPkt->ResultCode == 3)
				{
					MyPlayerID = ResPkt->PlayerID;
				}

				// UI(블루프린트) 쪽으로 이벤트 쫙 쏴주기!
				OnLoginResult.Broadcast((int32)ResPkt->ResultCode);
			}
			else if (Header->Id == S_TO_C_MATCH_SUCCESS)
			{
				UE_LOG(LogTemp, Warning, TEXT("매칭 성공! 인게임 맵으로 이동합니다."));
				OnMatchSuccess.Broadcast(); // 로비 위젯에게 알려줌
			}

			ProcessedBytes += Header->Size;
		}
	}
}

void USFGameInstance::SendChatMessage(FString Message)
{
	PacketChat Pkt = {};
	Pkt.Size = sizeof(PacketChat);
	Pkt.Id = C_TO_S_CHAT;

	// FString(언리얼 한글) -> UTF8(서버용) -> char 배열로 안전하게 복사
	strncpy_s(Pkt.Message, TCHAR_TO_UTF8(*Message), sizeof(Pkt.Message) - 1);

	SendPacket(&Pkt, sizeof(Pkt));
}

void USFGameInstance::SendHitReq(int32 VictimID, int32 Damage)
{
	PacketHitReq Pkt = {};

	Pkt.Size = sizeof(PacketHitReq);
	Pkt.Id = C_TO_S_HIT_REQ;

	Pkt.VictimID = VictimID;
	Pkt.Damage = Damage;

	SendPacket(&Pkt, sizeof(Pkt));

	UE_LOG(LogTemp, Warning, TEXT("[Hit Req 전송] 앗싸 명중! 맞은 유저: %d / 데미지: %d"), VictimID, Damage);
}

void USFGameInstance::SendSkillPacket(int32 SkillIndex, FVector TargetLoc)
{
	PacketPlayerSkill Pkt = {};
	Pkt.Size = sizeof(PacketPlayerSkill);
	Pkt.Id = C_TO_S_PLAYER_SKILL;
	Pkt.PlayerID = MyPlayerID;
	Pkt.SkillIndex = SkillIndex;

	Pkt.TargetX = TargetLoc.X;
	Pkt.TargetY = TargetLoc.Y;
	Pkt.TargetZ = TargetLoc.Z;

	SendPacket(&Pkt, sizeof(Pkt));
	UE_LOG(LogTemp, Log, TEXT("스킬 패킷 전송! (Index: %d)"), SkillIndex);
}

void USFGameInstance::RequestMatchmaking(int32 ClassType)
{
	MyClassType = ClassType; // 금고에 직업 저장

	PacketMatchReq Pkt;
	Pkt.Size = sizeof(PacketMatchReq);
	Pkt.Id = C_TO_S_MATCH_REQ;
	Pkt.ClassType = ClassType;

	SendPacket(&Pkt, sizeof(Pkt));
	UE_LOG(LogTemp, Warning, TEXT("서버에 매칭 요청을 보냈습니다. 대기 중..."));
}

void USFGameInstance::RequestCancelMatchmaking()
{
	PacketMatchCancelReq Pkt;
	Pkt.Size = sizeof(PacketMatchCancelReq);
	Pkt.Id = C_TO_S_MATCH_CANCEL_REQ;

	SendPacket(&Pkt, sizeof(Pkt));
	UE_LOG(LogTemp, Warning, TEXT("매칭을 취소했습니다."));
}
