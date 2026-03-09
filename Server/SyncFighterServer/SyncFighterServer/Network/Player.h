#pragma once
#include <cstdint>

class Session;

class Player
{
public:
	// 생성될 때 어떤 세션의 주인인지, 직업이 뭔지 받습니다.
	Player(Session* ownerSession, int32_t classType);
	~Player() = default;

	void InitStats();

	// 스킬 계수를 받아 최종 데미지를 계산하는 함수
	int32_t CalculateDamage(float skillMultiplier);

public:
	Session* _session; // 내 패킷을 보내줄 네트워크 세션
	int32_t _classType;

	// --- RPG 기본 스탯 ---
	int32_t _maxHp;
	int32_t _hp;
	int32_t _attackPower; // 기본 공격력

	// --- 상태 및 위치 ---
	float _x, _y, _z, _yaw;
	bool _isDead;
	int32_t _kills;
	int32_t _deaths;
};

