#include "GameRoom.h"

// 전역 객체 실체화 (Linker Error 방지)
GameRoom GGameRoom;

GameRoom::GameRoom()
{
    _spawnPoints.push_back({ 900.0f, 1000.0f, 100.0f, 0.0f }); // 1번 자리
    _spawnPoints.push_back({ 2100.0f, 1000.0f, 100.0f, 180.0f }); // 2번 자리
}

void GameRoom::Enter(Session* session)
{
	std::lock_guard<std::mutex> lock(_lock);
	_sessions.push_back(session);

    SpawnPoint spawn = GetRandomSpawnPoint();
    session->_x = spawn.x;
    session->_y = spawn.y;
    session->_z = spawn.z;
    session->_yaw = spawn.yaw;

    PacketLogin loginPacket;
    loginPacket.Size = sizeof(PacketLogin);
    loginPacket.Id = S_TO_C_LOGIN; // 0번
    loginPacket.MyPlayerID = session->_id;
    loginPacket.SpawnX = session->_x;
    loginPacket.SpawnY = session->_y;
    loginPacket.SpawnZ = session->_z;

    session->Send(&loginPacket, sizeof(loginPacket));

	std::cout << "User " << session->_id << " Entered Room. Total: " << _sessions.size() << std::endl;
}

void GameRoom::Leave(Session* session)
{
	std::lock_guard<std::mutex> lock(_lock);

	// 해당 세션 찾아서 삭제
	for (auto it = _sessions.begin(); it != _sessions.end(); ++it)
	{
		if (*it == session)
		{
			_sessions.erase(it);
			break;
		}
	}
}

void GameRoom::Broadcast(void* packet, int32_t size, Session* exceptMe)
{
	std::lock_guard<std::mutex> lock(_lock);

	for (Session* session : _sessions)
	{
		if (session == exceptMe) continue;
		session->Send(packet, size);
	}
}

void GameRoom::HandleAttack(Session* attacker)
{
    std::lock_guard<std::mutex> lock(_lock);

    float attackRange = 150.0f; // 공격 사거리 (1.5m)
    int32_t damage = 10;        // 공격력

    for (Session* victim : _sessions)
    {
        if (victim == attacker) continue; // 자해 방지
        if (victim->_isDead) continue;    // 시체 훼손 방지

        // 거리 계산: sqrt((x2-x1)^2 + (y2-y1)^2)
        // (성능을 위해 sqrt 없이 거리제곱만 비교하는 게 좋지만, 지금은 직관적으로)
        float dx = victim->_x - attacker->_x;
        float dy = victim->_y - attacker->_y;
        float dist = std::sqrt(dx * dx + dy * dy);

        // 사거리 안에 들어왔다면? -> 히트!
        if (dist <= attackRange)
        {
            // 1. HP 깎기
            victim->_hp -= damage;
            if (victim->_hp <= 0) 
            {
                victim->_hp = 0;
                victim->_isDead = true;

                std::thread([this, victim]() {
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    this->Respawn(victim); // 5초 뒤 부활 함수 호출
                    }).detach();
            }

            std::cout << "[Hit!] User " << attacker->_id << " -> User " << victim->_id
                << " (HP: " << victim->_hp << ")" << std::endl;

            // 2. 데미지 패킷 보내기 (모두에게 알려야 피격 모션 재생 가능)
            PacketDamage damagePacket;
            damagePacket.Size = sizeof(PacketDamage);
            damagePacket.Id = S_TO_C_DAMAGE; // 3번
            damagePacket.AttackerID = attacker->_id;
            damagePacket.VictimID = victim->_id;
            damagePacket.DamageAmount = damage;
            damagePacket.RemainingHP = victim->_hp;

            // Broadcast: 모두에게 "제 맞았어!" 알림
            // (여기선 exceptMe 없이 모두에게 보냄. 맞은 놈도 알아야 하니까)
            for (Session* s : _sessions)
            {
                s->Send(&damagePacket, sizeof(damagePacket));
            }
        }
    }
}

void GameRoom::Respawn(Session* session)
{
    std::lock_guard<std::mutex> lock(_lock);

    // 이미 나간 유저면 패스
    // (세션 포인터 유효성 체크가 필요하지만, 지금은 간단히직접 접근)
    // *주의: 실제로는 SessionID로 맵에서 찾아야 안전함!

    session->_hp = 100;
    session->_isDead = false;

    SpawnPoint spawn = GetRandomSpawnPoint();
    session->_x = spawn.x;
    session->_y = spawn.y;
    session->_z = spawn.z;

    // 부활 패킷 전송
    PacketRespawn respawnPkt;
    respawnPkt.Size = sizeof(PacketRespawn);
    respawnPkt.Id = S_TO_C_RESPAWN; // 4번
    respawnPkt.PlayerID = session->_id;
    respawnPkt.X = session->_x;
    respawnPkt.Y = session->_y;
    respawnPkt.Z = session->_z;

    // 부활 패킷 브로드캐스트
    for (Session* s : _sessions)
    {
        s->Send(&respawnPkt, sizeof(respawnPkt));
    }

    std::cout << "User " << session->_id << " Respawned!" << std::endl;
}

SpawnPoint GameRoom::GetRandomSpawnPoint()
{
    if (_spawnPoints.empty()) return { 900, 1000, 100, 0 }; // 비상용 기본값

    int index = rand() % _spawnPoints.size();
    return _spawnPoints[index];
}
