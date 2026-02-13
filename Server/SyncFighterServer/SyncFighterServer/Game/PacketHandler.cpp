#include "PacketHandler.h"
#include "GameRoom.h" // GameRoom을 알아야 브로드캐스팅을 하니까
#include <cmath>

void PacketHandler::HandlePacket(Session* session, char* buffer, int32_t len)
{
	PacketHeader* header = (PacketHeader*)buffer;

	// 스위치문이 여기 숨어있지만, 메인 함수는 깔끔해집니다.
	// (나중엔 이것도 함수 포인터 배열로 바꿀 수 있습니다)
	switch (header->Id)
	{
	case C_TO_S_PLAYER_MOVE:
		HandlePlayerMove(session, (PacketPlayerMove*)buffer);
		break;
	case C_TO_S_PLAYER_ATTACK:
		HandlePlayerAttack(session, (PacketPlayerAttack*)buffer);
		break;
	}
	
}

void PacketHandler::HandlePlayerMove(Session* session, PacketPlayerMove* packet)
{
	session->_x = packet->X;
	session->_y = packet->Y;
	session->_z = packet->Z;
	session->_yaw = packet->Yaw;

	// 패킷 검증 및 수정
	packet->PlayerID = session->_id;
	// 방에 있는 사람들에게 전송
	GGameRoom.Broadcast(packet, packet->Size, session);
}

void PacketHandler::HandlePlayerAttack(Session* session, PacketPlayerAttack* packet)
{
	packet->PlayerID = session->_id;
	std::cout << "[User " << session->_id << "] Attack!" << std::endl;

	// 방에 뿌리기
	GGameRoom.Broadcast(packet, packet->Size, session);

	GGameRoom.HandleAttack(session);
}