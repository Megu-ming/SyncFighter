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