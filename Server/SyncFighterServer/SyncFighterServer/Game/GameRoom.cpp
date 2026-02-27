#include "GameRoom.h"

// 전역 객체 실체화 (Linker Error 방지)
GameRoom GGameRoom;

GameRoom::GameRoom()
{
    _spawnPoints.push_back({ 900.0f, 1000.0f, 100.0f, 0.0f }); // 1번 자리
    _spawnPoints.push_back({ 2100.0f, 1000.0f, 100.0f, 180.0f }); // 2번 자리
}

Session* GameRoom::FindSession(int32_t id)
{
    std::lock_guard<std::mutex> lock(_lock);

    for (Session* s : _sessions) {
        if (s->_id == id) return s;
    }
    return nullptr;
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

void GameRoom::Respawn(int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(_lock);

    // 이미 나간 유저면 패스
    // (세션 포인터 유효성 체크가 필요하지만, 지금은 간단히직접 접근)
    // *주의: 실제로는 SessionID로 맵에서 찾아야 안전함!
    Session* targetSession = nullptr;
    for (Session* s : _sessions)
    {
        if (s->_id == sessionId)
        {
            targetSession = s;
            break;
        }
    }

    if (targetSession == nullptr)
    {
        std::cout << "User " << sessionId << " already left. Cancel Respawn." << std::endl;
        return;
    }

    targetSession->_hp = 100;
    targetSession->_isDead = false;

    SpawnPoint spawn = GetRandomSpawnPoint();
    targetSession->_x = spawn.x;
    targetSession->_y = spawn.y;
    targetSession->_z = spawn.z;

    // 부활 패킷 전송
    PacketRespawn respawnPkt;
    respawnPkt.Size = sizeof(PacketRespawn);
    respawnPkt.Id = S_TO_C_RESPAWN; // 4번
    respawnPkt.PlayerID = targetSession->_id;
    respawnPkt.X = targetSession->_x;
    respawnPkt.Y = targetSession->_y;
    respawnPkt.Z = targetSession->_z;

    // 부활 패킷 브로드캐스트
    for (Session* s : _sessions)
    {
        s->Send(&respawnPkt, sizeof(respawnPkt));
    }

    std::cout << "User " << targetSession->_id << " Respawned!" << std::endl;
}

SpawnPoint GameRoom::GetRandomSpawnPoint()
{
    if (_spawnPoints.empty()) return { 900, 1000, 100, 0 }; // 비상용 기본값

    int index = rand() % _spawnPoints.size();
    return _spawnPoints[index];
}
