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

	C_TO_S_LOGIN_REQ = 10,    // C -> S 해당 아이디로 로그인 요청
	S_TO_C_LOGIN_RES = 11,    // S -> C 로그인 결과
	C_TO_S_REGISTER_REQ = 12, // C -> S 새로운 계정 생성
	C_TO_S_ENTER_GAME_REQ = 13, // C -> S 직업 선택 후 게임 입장 요청

	C_TO_S_CHAT = 20,		// C -> S 채팅 패킷
	S_TO_C_CHAT = 21,		// S -> C 채팅 패킷
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

// 로그인/가입 결과 코드
enum ELoginResult : int32_t
{
	SUCCESS = 0,               // 성공 (방으로 입장)
	FAIL_WRONG_PW = 1,         // 실패: 비밀번호 틀림
	FAIL_NOT_FOUND = 2,        // 실패: 없는 아이디 (클라는 이 코드를 받으면 '생성하시겠습니까?' UI 띄움)
	REGISTER_SUCCESS = 3,      // 회원가입 성공
	REGISTER_FAIL_DUP = 4      // 회원가입 실패: 이미 있는 아이디 (동시접속 예외처리)
};

enum EClassType : int32_t
{
	WARRIOR = 0,
	MAGE = 1
};

#pragma region 게임 진입 전 패킷
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
	int32_t PlayerID; // 성공했을 때 부여받을 고유 번호
};

// 3. 회원가입 요청 패킷 (Client -> Server)
struct PacketRegisterReq : public PacketHeader
{
	char UserID[32];
	char Password[32];
};

struct PacketEnterGameReq : public PacketHeader
{
	int32_t ClassType; // 선택한 직업 번호를 들고 게임 입장
};
#pragma endregion


#pragma region 캐릭터 조작 패킷
struct PacketPlayerMove : public PacketHeader
{
	int32_t PlayerID;
	int32_t ClassType;
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
#pragma endregion

struct PacketChat : public PacketHeader
{
	int32_t SenderId;       // 누가 보냈는지
	char Message[256];		// 채팅 내용
};

#pragma pack(pop)