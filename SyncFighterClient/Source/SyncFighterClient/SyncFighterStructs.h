#pragma once
#include "CoreMinimal.h"

#pragma pack(push, 1)

enum PacketID : int32
{
	S_TO_C_LOGIN = 0,
	C_TO_S_PLAYER_MOVE = 1,
	C_TO_S_PLAYER_ATTACK = 2,
	S_TO_C_DAMAGE = 3,
	S_TO_C_RESPAWN = 4,

	C_TO_S_LOGIN_REQ = 10,    // C -> S 해당 아이디로 로그인 요청
	S_TO_C_LOGIN_RES = 11,    // S -> C 로그인 결과
	C_TO_S_REGISTER_REQ = 12, // C -> S 새로운 계정 생성

	C_TO_S_CHAT = 20, // 추가
	S_TO_C_CHAT = 21, // 추가
};

struct PacketHeader
{
	int32 Size;
	int32 Id;  
};

struct PacketLogin : public PacketHeader
{
	int32 MyPlayerID; // "너는 이제부터 이 번호다"
	float SpawnX, SpawnY, SpawnZ; // "여기서 태어나라"
};

// 로그인/가입 결과 코드
enum ELoginResult : int32
{
	SUCCESS = 0,               // 성공 (방으로 입장)
	FAIL_WRONG_PW = 1,         // 실패: 비밀번호 틀림
	FAIL_NOT_FOUND = 2,        // 실패: 없는 아이디 (클라는 이 코드를 받으면 '생성하시겠습니까?' UI 띄움)
	REGISTER_SUCCESS = 3,      // 회원가입 성공
	REGISTER_FAIL_DUP = 4      // 회원가입 실패: 이미 있는 아이디 (동시접속 예외처리)
};

// 1. 로그인 요청 패킷 (Client -> Server)
struct PacketLoginReq : public PacketHeader
{
	char UserID[32];
	char Password[32];
};

// 2. 로그인 응답 패킷 (Server -> Client)
struct PacketLoginRes : public PacketHeader
{
	ELoginResult ResultCode;
	int32 PlayerID; // 성공했을 때 부여받을 고유 번호
};

// 3. 회원가입 요청 패킷 (Client -> Server)
struct PacketRegisterReq : public PacketHeader
{
	char UserID[32];
	char Password[32];
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

struct PacketChat : public PacketHeader
{
	int32 SenderId;       // 누가 보냈는지
	char Message[256];    // 채팅 내용
};
#pragma pack(pop)