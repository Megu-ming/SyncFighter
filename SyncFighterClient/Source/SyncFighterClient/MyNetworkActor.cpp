#include "MyNetworkActor.h"
#include "SFGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SyncFighterStructs.h" // 패킷 구조체 정의 포함
#include "Components/WidgetComponent.h"
#include "UI/SFHPBarWidget.h"
#include "Player/SFCharacter.h"

// Sets default values
AMyNetworkActor::AMyNetworkActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMyNetworkActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMyNetworkActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMyNetworkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. GameInstance 가져오기 (이제 소켓은 얘가 관리함)
	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI == nullptr) return;

	//// --- [0. 공격 테스트 (마우스 왼쪽 클릭)] ---
	//APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	//if (PC && PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	//{
	//	PacketPlayerAttack AttackPacket;
	//	AttackPacket.Size = sizeof(PacketPlayerAttack);
	//	AttackPacket.ID = C_TO_S_PLAYER_ATTACK;
	//	AttackPacket.PlayerID = 0;

	//	// [수정] BytesSent 변수 제거, 크기만 전달
	//	GI->SendPacket(&AttackPacket, sizeof(AttackPacket));

	//	UE_LOG(LogTemp, Warning, TEXT("공격 패킷 전송!"));

	//	ACharacter* MyCharacter = Cast<ACharacter>(PC->GetPawn());
	//	if (MyCharacter && AttackMontage)
	//	{
	//		MyCharacter->PlayAnimMontage(AttackMontage);
	//	}
	//}

	// --- [1. 이동 패킷 보내기 (Send)] --- 
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		FVector Location = PlayerPawn->GetActorLocation();
		FRotator Rotation = PlayerPawn->GetActorRotation();

		PacketPlayerMove MovePacket;
		MovePacket.Size = sizeof(PacketPlayerMove);
		MovePacket.ID = C_TO_S_PLAYER_MOVE;
		MovePacket.PlayerID = 0;
		MovePacket.X = Location.X;
		MovePacket.Y = Location.Y;
		MovePacket.Z = Location.Z;
		MovePacket.Yaw = Rotation.Yaw;

		// [수정] BytesSent 변수 제거, 크기만 전달
		GI->SendPacket(&MovePacket, sizeof(MovePacket));
	}

	// --- [2. 받기 (Recv) & 처리] ---
	// 소켓 직접 접근(Socket->Recv) 대신 GI에게 데이터 있냐고 물어봅니다.
	TArray<uint8> RecvBuffer;
	if (GI->HandleRecv(RecvBuffer))
	{
		uint8* Buffer = RecvBuffer.GetData();
		int32 BytesRead = RecvBuffer.Num();
		int32 ProcessedBytes = 0;

		// 패킷 파싱 루프
		while (ProcessedBytes + sizeof(PacketHeader) <= (uint32)BytesRead)
		{
			// 일단 헤더 부분을 읽어서 크기 확인
			PacketHeader* Header = (PacketHeader*)(Buffer + ProcessedBytes);

			// 데이터가 짤려서 왔으면(패킷 크기보다 남은 데이터가 적으면) 중단하고 다음 틱에 처리
			// (완벽한 처리를 위해선 RingBuffer가 필요하지만, 일단 현재 구조 유지)
			if (ProcessedBytes + Header->Size > BytesRead)
			{
				break;
			}

			// 헤더의 ID에 따라 분기 처리
			if (Header->ID == S_TO_C_LOGIN)
			{
				PacketLogin* LoginPkt = (PacketLogin*)(Buffer + ProcessedBytes);

				// GameInstance에 내 ID 저장
				GI->MyPlayerID = LoginPkt->MyPlayerID;

				UE_LOG(LogTemp, Warning, TEXT("접속 성공! 내 ID는 [%d]번 입니다."), GI->MyPlayerID);
			}
			else if (Header->ID == C_TO_S_PLAYER_MOVE)
			{
				PacketPlayerMove* Packet = (PacketPlayerMove*)(Buffer + ProcessedBytes);

				// 안전장치
				if (Packet->PlayerID < 0 || Packet->PlayerID > 1000)
				{
					ProcessedBytes += Packet->Size;
					continue;
				}

				if (RemotePlayers.Contains(Packet->PlayerID))
				{
					FRemotePlayerInfo& Info = RemotePlayers[Packet->PlayerID];
					Info.TargetPos = FVector(Packet->X, Packet->Y, Packet->Z);
					Info.TargetRot = FRotator(0, Packet->Yaw, 0);
				}
				else
				{
					// 신규 유저 소환
					if (RemoteCharacterClass)
					{
						FVector SpawnLoc(Packet->X, Packet->Y, Packet->Z);
						FRotator SpawnRot(0, Packet->Yaw, 0);
						FActorSpawnParameters SpawnParams;
						ASFCharacter* NewActor = GetWorld()->SpawnActor<ASFCharacter>(RemoteCharacterClass, SpawnLoc, SpawnRot, SpawnParams);

						if (NewActor)
						{
							FRemotePlayerInfo NewInfo;
							NewInfo.Character = NewActor;
							NewInfo.TargetPos = SpawnLoc;
							NewInfo.TargetRot = SpawnRot;
							RemotePlayers.Add(Packet->PlayerID, NewInfo);

							ACharacter* NewChar = Cast<ACharacter>(NewActor);
							if (NewChar)
							{
								NewChar->SetReplicates(false);
								NewChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
							}
						}
					}
				}
			}
			else if (Header->ID == C_TO_S_PLAYER_ATTACK)
			{
				PacketPlayerAttack* AttackPkt = (PacketPlayerAttack*)(Buffer + ProcessedBytes);

				if (RemotePlayers.Contains(AttackPkt->PlayerID))
				{
					FRemotePlayerInfo& Info = RemotePlayers[AttackPkt->PlayerID];
					UE_LOG(LogTemp, Warning, TEXT("[User %d] 공격 시전!"), AttackPkt->PlayerID);

					ASFCharacter* Character = Info.Character;
					if (Character)
					{
						Character->ProcessAttack();
					}
				}
			}
			else if (Header->ID == S_TO_C_DAMAGE)
			{
				PacketDamage* DmgPkt = (PacketDamage*)(Buffer + ProcessedBytes);

				// 1. 대상 찾기
				ASFCharacter* TargetChar = nullptr;
				if (DmgPkt->VictimID == GI->MyPlayerID)
				{
					// 나 자신
					TargetChar = Cast<ASFCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
				}
				else if (RemotePlayers.Contains(DmgPkt->VictimID))
				{
					// 적
					TargetChar = Cast<ASFCharacter>(RemotePlayers[DmgPkt->VictimID].Character);
				}

				if (TargetChar)
				{
					TargetChar->ProcessDamage(DmgPkt->RemainingHP);
				}
			}
			else if (Header->ID == S_TO_C_RESPAWN) // 4번
			{
				PacketRespawn* ResPkt = (PacketRespawn*)(Buffer + ProcessedBytes);

				ASFCharacter* TargetChar = nullptr;
				if (ResPkt->PlayerID == GI->MyPlayerID)
				{
					// 나 자신
					TargetChar = Cast<ASFCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
				}
				else if (RemotePlayers.Contains(ResPkt->PlayerID))
				{
					// 적
					TargetChar = Cast<ASFCharacter>(RemotePlayers[ResPkt->PlayerID].Character);
				}

				if (TargetChar)
				{
					TargetChar->ProcessRespawn(FVector(ResPkt->X, ResPkt->Y, ResPkt->Z));
				}
			}

			// 다음 패킷으로 포인터 이동
			ProcessedBytes += Header->Size;
		}
	}

	// --- [3. 보간 (Interpolation)] ---
	// (기존과 동일, 가짜 속도 주입 부분은 지난번에 애니메이션 BP에서 해결했으면 제거해도 됨)
	for (auto& Elem : RemotePlayers)
	{
		FRemotePlayerInfo& Info = Elem.Value;
		if (Info.Character && IsValid(Info.Character))
		{
			FVector OldPos = Info.Character->GetActorLocation();
			FVector NewPos = FMath::VInterpTo(OldPos, Info.TargetPos, DeltaTime, 15.0f);
			FRotator NewRot = FMath::RInterpTo(Info.Character->GetActorRotation(), Info.TargetRot, DeltaTime, 15.0f);

			Info.Character->SetActorLocation(NewPos);
			Info.Character->SetActorRotation(NewRot);
		}
	}
}