#include "MyNetworkActor.h"
#include "SFGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SyncFighterStructs.h" // 패킷 구조체 정의 포함
#include "Components/WidgetComponent.h"
#include "UI/SFHPBarWidget.h"
#include "Player/SFCharacter.h"
#include "Player/SFMage.h"
#include "Player/SFWarrior.h"
#include "Player/State/SFPlayerState.h"
#include "UI/HUD/SFHUDWidget.h"
#include "UI/SFResultWidget.h"

// Sets default values
AMyNetworkActor::AMyNetworkActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void AMyNetworkActor::BeginPlay()
{
	Super::BeginPlay();

	USFGameInstance* GI = Cast<USFGameInstance>(GetGameInstance());
	if (GI)
	{
		// 내가 선택한 직업에 맞는 클래스 준비
		TSubclassOf<ASFCharacter> MySpawnClass = (GI->MyClassType == 1) ? MageClass : WarriorClass;

		if (MySpawnClass)
		{
			// (나중에 PlayerStart 위치로 바꾸시면 됩니다)
			FVector SpawnLoc(900.0f, 1000.0f, 100.0f);
			FRotator SpawnRot(0.0f, 0.0f, 0.0f);

			// 1. 소환!
			ASFCharacter* MyChar = GetWorld()->SpawnActor<ASFCharacter>(MySpawnClass, SpawnLoc, SpawnRot);
			MyChar->PlayerID = GI->MyPlayerID;
			MyChar->InitStatus(GI->MyClassType);

			// 2. 빙의! (키보드/마우스 컨트롤 연결)
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (PC && MyChar)
			{
				PC->Possess(MyChar);
				
				FInputModeGameOnly InputMode;
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = false;

				UE_LOG(LogTemp, Warning, TEXT("내 캐릭터 스폰 및 빙의 완료! (Class: %d)"), GI->MyClassType);
			}
		}

		GI->RequestEnterGame(GI->MyClassType);
	}

	// HUD 위젯 생성 및 화면에 띄우기
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<USFHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->UpdateTimer(180);
			HUDWidget->UpdateMyScore(0, 0);
			HUDWidget->UpdateEnemyScore(0, 0);
		}
	}
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

	// --- [1. 이동 패킷 보내기 (Send)] --- 
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		FVector Location = PlayerPawn->GetActorLocation();
		FRotator Rotation = PlayerPawn->GetActorRotation();

		PacketPlayerMove MovePacket;
		MovePacket.Size = sizeof(PacketPlayerMove);
		MovePacket.Id = C_TO_S_PLAYER_MOVE;
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
			if (Header->Id == S_TO_C_LOGIN)
			{
				PacketLogin* LoginPkt = (PacketLogin*)(Buffer + ProcessedBytes);

				// GameInstance에 내 ID 저장
				GI->MyPlayerID = LoginPkt->MyPlayerID;

				UE_LOG(LogTemp, Warning, TEXT("접속 성공! 내 ID는 [%d]번 입니다."), GI->MyPlayerID);
			}
			else if (Header->Id == S_TO_C_GAME_OVER)
			{
				PacketGameOver* OverPkt = (PacketGameOver*)(Buffer + ProcessedBytes);

				// 1. 내 캐릭터 조작 막기 (싸움 강제 중지)
				APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
				APawn* MyPawn = PC ? PC->GetPawn() : nullptr;
				if (PC && MyPawn)
				{
					MyPawn->DisableInput(PC);
					// 결과창 클릭을 위해 마우스 커서 표시
					FInputModeUIOnly UIInputMode;
					PC->SetInputMode(UIInputMode);
					PC->bShowMouseCursor = true;
				}

				// 2. 내 PlayerState에서 최종 점수 가져오기
				ASFPlayerState* MyPS = Cast<ASFPlayerState>(PC->PlayerState);
				int32 FinalKills = MyPS ? MyPS->Kills : 0;
				int32 FinalDeaths = MyPS ? MyPS->Deaths : 0;

				// 3. 결과창 띄우기
				bool bIsWinner = (OverPkt->WinnerPlayerID == GI->MyPlayerID); // 내가 승리자인지 판별

				if (ResultWidgetClass)
				{
					ResultWidget = CreateWidget<USFResultWidget>(GetWorld(), ResultWidgetClass);
					if (ResultWidget)
					{
						ResultWidget->AddToViewport();
						ResultWidget->SetResultInfo(bIsWinner, FinalKills, FinalDeaths);
					}
				}

				UE_LOG(LogTemp, Warning, TEXT("게임 오버! 승리자 ID: %d"), OverPkt->WinnerPlayerID);
			}
			else if (Header->Id == S_TO_C_TIMER_SYNC)
			{
				PacketTimerSync* SyncPkt = (PacketTimerSync*)(Buffer + ProcessedBytes);

				if (HUDWidget)
				{
					// 서버에서 받은 정확한 시간으로 화면의 UI를 갱신
					HUDWidget->UpdateTimer(SyncPkt->RemainingTime);
				}
			}
			else if (Header->Id == C_TO_S_PLAYER_MOVE)
			{
				PacketPlayerMove* Packet = (PacketPlayerMove*)(Buffer + ProcessedBytes);

				// 안전장치
				if (Packet->PlayerID < 0 || Packet->PlayerID > 1000)
				{
					ProcessedBytes += Header->Size;
					continue;
				}

				if (Packet->PlayerID == GI->MyPlayerID)
				{
					ProcessedBytes += Header->Size;
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
					TSubclassOf<ASFCharacter> SpawnClass = (Packet->ClassType == 1) ? MageClass : WarriorClass;
					// 신규 유저 소환
					if (SpawnClass)
					{
						FVector SpawnLoc(Packet->X, Packet->Y, Packet->Z);
						FRotator SpawnRot(0, Packet->Yaw, 0);
						FActorSpawnParameters SpawnParams;
						ASFCharacter* NewChar = GetWorld()->SpawnActor<ASFCharacter>(SpawnClass, SpawnLoc, SpawnRot, SpawnParams);
						ASFPlayerState* NewPS = GetWorld()->SpawnActor<ASFPlayerState>();

						if (NewChar && NewPS)
						{
							NewPS->SFPlayerID = Packet->PlayerID;
							NewChar->InitStatus(Packet->ClassType);

							FRemotePlayerInfo NewInfo;
							NewInfo.Character = NewChar;
							NewInfo.PlayerState = NewPS;
							NewInfo.TargetPos = SpawnLoc;
							NewInfo.TargetRot = SpawnRot;
							RemotePlayers.Add(Packet->PlayerID, NewInfo);

							NewChar->PlayerID = Packet->PlayerID;
							NewChar->SetReplicates(false);
							NewChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
						}
					}
				}
			}
			else if (Header->Id == C_TO_S_PLAYER_ATTACK)
			{
				PacketPlayerAttack* AttackPkt = (PacketPlayerAttack*)(Buffer + ProcessedBytes);

				if (AttackPkt->PlayerID == GI->MyPlayerID)
				{
					ProcessedBytes += Header->Size;
					continue;
				}

				if (RemotePlayers.Contains(AttackPkt->PlayerID))
				{
					FRemotePlayerInfo& Info = RemotePlayers[AttackPkt->PlayerID];
					UE_LOG(LogTemp, Warning, TEXT("[User %d] 공격 시전!"), AttackPkt->PlayerID);

					ASFCharacter* Character = Info.Character;
					if (Character)
					{
						Character->ProcessBasicAttack();
					}
				}
			}
			else if (Header->Id == C_TO_S_PLAYER_SKILL)
			{
				PacketPlayerSkill* SkillPkt = (PacketPlayerSkill*)(Buffer + ProcessedBytes);

				if (SkillPkt->PlayerID == GI->MyPlayerID)
				{
					ProcessedBytes += Header->Size;
					continue;
				}

				if (RemotePlayers.Contains(SkillPkt->PlayerID))
				{
					ASFCharacter* Caster = RemotePlayers[SkillPkt->PlayerID].Character;

					ASFMage* MageChar = Cast<ASFMage>(Caster);
					if (MageChar)
					{
						if (SkillPkt->SkillIndex == 0) // 0번(Q스킬) 이라면!
						{
							FVector TargetLoc(SkillPkt->TargetX, SkillPkt->TargetY, SkillPkt->TargetZ);
							MageChar->PlayRemoteSkillQ(TargetLoc);
						}
						else
						{
							FVector TargetLoc(SkillPkt->TargetX, SkillPkt->TargetY, SkillPkt->TargetZ);
							MageChar->PlayRemoteSkillE(TargetLoc);
						}
					}
					else
					{
						ASFWarrior* WarriorChar = Cast<ASFWarrior>(Caster);
						if (SkillPkt->SkillIndex == 0)
						{
							FVector TargetLoc(SkillPkt->TargetX, SkillPkt->TargetY, SkillPkt->TargetZ);
							WarriorChar->PlayRemoteSkillQ(TargetLoc);
						}
						else
						{
							WarriorChar->ProcessSkillE();
						}
					}
				}
			}
			else if (Header->Id == S_TO_C_DAMAGE)
			{
				PacketDamage* DmgPkt = (PacketDamage*)(Buffer + ProcessedBytes);

				UE_LOG(LogTemp, Warning, TEXT("[Damage Packet] 맞은사람 ID: %d / 내 ID(GI): %d / 남은체력: %d"), DmgPkt->VictimID, GI->MyPlayerID, DmgPkt->RemainingHP);

				ASFCharacter* VictimChar = nullptr;
				ASFPlayerState* VictimPS = nullptr;
				ASFPlayerState* AttackerPS = nullptr;

				// 1. 대상 찾기
				if (DmgPkt->VictimID == GI->MyPlayerID)
				{
					// 나 자신
					UE_LOG(LogTemp, Error, TEXT("-> 앗! 내 캐릭터가 맞았다!")); // 빨간 글씨로 눈에 띄게!
					VictimChar = Cast<ASFCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
					// 내 PlayerState는 기본 컨트롤러에 붙어있음
					VictimPS = Cast<ASFPlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
				}
				else if (RemotePlayers.Contains(DmgPkt->VictimID))
				{
					// 적
					VictimChar = RemotePlayers[DmgPkt->VictimID].Character;
					VictimPS = RemotePlayers[DmgPkt->VictimID].PlayerState;
				}

				if (DmgPkt->AttackerID == GI->MyPlayerID)
				{
					AttackerPS = Cast<ASFPlayerState>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerState);
				}
				else if (RemotePlayers.Contains(DmgPkt->AttackerID))
				{
					AttackerPS = RemotePlayers[DmgPkt->AttackerID].PlayerState;
				}

				if (VictimChar)
				{
					VictimChar->ProcessDamage(DmgPkt->DamageAmount, DmgPkt->RemainingHP);
					if (AttackerPS) AttackerPS->AddDamage(DmgPkt->DamageAmount);

					if (DmgPkt->RemainingHP <= 0)
					{
						VictimChar->ProcessDeath();
						if (VictimPS) VictimPS->AddDeath();   // 죽은 사람 데스 증가
						if (AttackerPS) AttackerPS->AddKill(); // 때린 사람 킬 증가

						if (HUDWidget)
						{
							// 내가 때린 거면 내 점수 UI 갱신!
							if (DmgPkt->AttackerID == GI->MyPlayerID && AttackerPS)
							{
								HUDWidget->UpdateMyScore(AttackerPS->Kills, AttackerPS->Deaths);
							}
							// 적이 날 때려죽인 거면 적 점수 UI 갱신!
							else if (AttackerPS)
							{
								HUDWidget->UpdateEnemyScore(AttackerPS->Kills, AttackerPS->Deaths);
							}

							// 내가 맞은 거면 내 데스 UI 갱신!
							if (DmgPkt->VictimID == GI->MyPlayerID && VictimPS)
							{
								HUDWidget->UpdateMyScore(VictimPS->Kills, VictimPS->Deaths);
							}
							// 적이 맞은 거면 적 데스 UI 갱신!
							else if (VictimPS)
							{
								HUDWidget->UpdateEnemyScore(VictimPS->Kills, VictimPS->Deaths);
							}
						}

						UE_LOG(LogTemp, Warning, TEXT("🔥 킬 발생! [%d]번 유저가 [%d]번 유저를 처치했습니다!"), DmgPkt->AttackerID, DmgPkt->VictimID);

						// 나중에 여기에 우측 상단 킬 로그 UI를 띄우는 이벤트를 추가
					}
				}
			}
			else if (Header->Id == S_TO_C_RESPAWN) // 4번
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
			else if (Header->Id == S_TO_C_CHAT)
			{
				PacketChat* ChatPkt = (PacketChat*)(Buffer + ProcessedBytes);

				// char 배열(UTF8) -> FString(언리얼 한글)으로 복구
				FString ReceivedMsg = FString(UTF8_TO_TCHAR(ChatPkt->Message));

				UE_LOG(LogTemp, Warning, TEXT("[Chat] %d번 유저: %s"), ChatPkt->SenderId, *ReceivedMsg);

				// GameInstance의 델리게이트를 통해 UI로 쫙 뿌려주기
				GI->OnChatReceived.Broadcast(ChatPkt->SenderId, ReceivedMsg);
			}

			// 다음 패킷으로 포인터 이동
			ProcessedBytes += Header->Size;
		}
	}

	// 보간
	for (auto& Elem : RemotePlayers)
	{
		FRemotePlayerInfo& Info = Elem.Value;

		// 캐릭터가 살아있고 유효한지 확인
		if (Info.Character && IsValid(Info.Character))
		{
			// 1. 목표 데이터 전달
			// (패킷 받을 때 넣어도 되지만, 확실하게 하기 위해 매번 갱신)
			Info.Character->TargetLocation = Info.TargetPos;
			Info.Character->TargetRotation = Info.TargetRot;

			// 2. 캐릭터 스스로 동기화하도록 명령
			Info.Character->SyncTransform(DeltaTime);
		}
	}
}