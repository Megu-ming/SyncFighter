#pragma once
#include "Protocol.h"

class Session;

class PacketHandler
{
public:
	static void HandlePacket(Session* session, char* buffer, int32_t len);

	// 각 패킷별 처리 함수
	static void HandlePlayerMove(Session* session, PacketPlayerMove* packet);
	static void HandlePlayerAttack(Session* session, PacketPlayerAttack* packet);
};