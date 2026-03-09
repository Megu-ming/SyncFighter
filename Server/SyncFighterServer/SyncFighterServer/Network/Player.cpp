#include "Player.h"
#include <cstdlib>

Player::Player(Session* ownerSession, int32_t classType)
	: _session(ownerSession), _classType(classType)
{
	InitStats();
}

void Player::InitStats()
{
	_isDead = false;
	_kills = 0;
	_deaths = 0;
	_x = 0; _y = 0; _z = 0; _yaw = 0;

	// 직업별 기본 스탯 세팅
	if (_classType == 0) // 전사 (쾅)
	{
		_maxHp = 500;
		_attackPower = 30; // 묵직한 공격력
	}
	else // 마법사 (기데온)
	{
		_maxHp = 300;
		_attackPower = 45; // 강력한 마법 공격력
	}

	_hp = _maxHp;
}

int32_t Player::CalculateDamage(float skillMultiplier)
{
	// 1. 기본 데미지 = 내 공격력 * 스킬 계수
	float baseDamage = _attackPower * skillMultiplier;

	// 2. 랜덤 계수 생성 (0.9 ~ 1.1 사이의 값)
	// rand() / RAND_MAX 는 0.0 ~ 1.0을 반환합니다.
	float randomFactor = 0.9f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f;

	// 3. 최종 데미지 산출 (소수점 버림)
	int32_t finalDamage = static_cast<int32_t>(baseDamage * randomFactor);

	// 데미지가 1보다 낮아지는 것을 방지 (최소 데미지 보정)
	if (finalDamage < 1) finalDamage = 1;

	return finalDamage;
}
