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

	UE_LOG(LogTemp, Log, TEXT("Login Request Sent: ID=%s"), *UserID);
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

			ProcessedBytes += Header->Size;
		}
	}
}
