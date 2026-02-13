#pragma once
#include "CoreMinimal.h" // 언리얼 타입을 위해 필요할 수 있음

#pragma pack(push, 1)

// C++ 표준 타입(int32_t) 대신 언리얼 타입(int32)을 써도 크기는 같습니다.
enum PacketID : int32
{
	S_TO_C_LOGIN = 0,
	C_TO_S_PLAYER_MOVE = 1,
	C_TO_S_PLAYER_ATTACK = 2,
	S_TO_C_DAMAGE = 3,
	S_TO_C_RESPAWN = 4,
};

struct PacketHeader
{
	int32 Size; // ★ 중요: 서버랑 똑같이 int32 (4바이트)여야 함!
	int32 ID;   // ★ 중요: 서버랑 똑같이 int32
};

struct PacketLogin : public PacketHeader
{
	int32 MyPlayerID; // "너는 이제부터 이 번호다"
	float SpawnX, SpawnY, SpawnZ; // "여기서 태어나라"
};

struct PacketPlayerMove : public PacketHeader
{
	int32 PlayerID;
	float X, Y, Z, Yaw;
};

struct PacketPlayerAttack : public PacketHeader
{
	int32 PlayerID;
};

struct PacketDamage : public PacketHeader
{
	int32 VictimID;    // 맞은 사람 ID
	int32 AttackerID;  // 때린 사람 ID
	int32 DamageAmount;// 깎인 체력
	int32 RemainingHP; // 남은 체력
};

struct PacketRespawn : public PacketHeader
{
	int32_t PlayerID;
	float X, Y, Z; // 부활 위치
};

#pragma pack(pop)