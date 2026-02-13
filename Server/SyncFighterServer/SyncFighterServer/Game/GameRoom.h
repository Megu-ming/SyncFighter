#pragma once
#include <vector>
#include <mutex>
#include "Session.h"

struct SpawnPoint
{
	float x, y, z, yaw;
};

class GameRoom
{
public:
	GameRoom();

	void Enter(Session* session); // 입장
	void Leave(Session* session); // 퇴장
	void Broadcast(void* packet, int32_t size, Session* exceptMe = nullptr); // 뿌리기

	void HandleAttack(Session* attacker);

	void Respawn(Session* session);
private:
	std::vector<Session*> _sessions;
	std::mutex _lock; // 이 방 전용 자물쇠

private:
	// 스폰 포인트 목록
	std::vector<SpawnPoint> _spawnPoints;

	// 랜덤하게 하나 뽑아오는 함수
	SpawnPoint GetRandomSpawnPoint();
};

// (임시) 전역으로 방 하나만 둠
extern GameRoom GGameRoom;