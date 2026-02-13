#pragma once
#include <cstdint>

#pragma pack(push, 1)

enum PacketID : int32_t // int32로 통일
{
	S_TO_C_LOGIN = 0,
	C_TO_S_PLAYER_MOVE = 1,
	C_TO_S_PLAYER_ATTACK = 2,
	S_TO_C_DAMAGE = 3,
	S_TO_C_RESPAWN = 4,
};

struct PacketHeader
{
	int32_t Size; // int32로 변경
	int32_t Id;   // int32로 변경
};

struct PacketLogin : public PacketHeader
{
	int32_t MyPlayerID;				// "너는 이제부터 이 번호다"
	float SpawnX, SpawnY, SpawnZ;	// "여기서 태어나라"
};

// 상속을 받으면 메모리 구조가 [Header][Body] 순서로 자동 배치됩니다.
struct PacketPlayerMove : public PacketHeader
{
	int32_t PlayerID;
	float X, Y, Z, Yaw;
};

struct PacketPlayerAttack : public PacketHeader
{
	int32_t PlayerID;
};

struct PacketDamage : public PacketHeader
{
	int32_t VictimID;    // 맞은 사람 ID
	int32_t AttackerID;  // 때린 사람 ID
	int32_t DamageAmount;// 깎인 체력
	int32_t RemainingHP; // 남은 체력
};

struct PacketRespawn : public PacketHeader
{
	int32_t PlayerID;
	float X, Y, Z; // 부활 위치
};

#pragma pack(pop)