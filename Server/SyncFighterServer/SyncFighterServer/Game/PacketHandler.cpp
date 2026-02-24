#include "PacketHandler.h"
#include "GameRoom.h" // GameRoom을 알아야 브로드캐스팅을 하니까
#include <cmath>
#include <unordered_map>
#include <string>
#include <cstring>

std::unordered_map<std::string, std::string> AccountDB = {
	{"test", "1234"},
	{"admin", "0000"}
};

int32_t GlobalPlayerIDCounter = 1;

void PacketHandler::HandlePacket(Session* session, char* buffer, int32_t len)
{
	PacketHeader* header = (PacketHeader*)buffer;

	switch (header->Id)
	{
	case C_TO_S_LOGIN_REQ: // 로그인 요청이 오면
		HandleLoginReq(session, (PacketLoginReq*)buffer);
		break;
	case C_TO_S_REGISTER_REQ: // 가입 요청이 오면
		HandleRegisterReq(session, (PacketRegisterReq*)buffer);
		break;
	case C_TO_S_PLAYER_MOVE:
		HandlePlayerMove(session, (PacketPlayerMove*)buffer);
		break;
	case C_TO_S_PLAYER_ATTACK:
		HandlePlayerAttack(session, (PacketPlayerAttack*)buffer);
		break;
    case C_TO_S_CHAT:
        HandleChat(session, (PacketChat*)buffer);
        break;
	}
}

// [로그인 요청 처리]
void PacketHandler::HandleLoginReq(Session* session, PacketLoginReq* packet)
{
    std::string requestID(packet->UserID);
    std::string requestPW(packet->Password);

    PacketLoginRes resPacket = {};
    resPacket.Size = sizeof(PacketLoginRes);
    resPacket.Id = S_TO_C_LOGIN_RES;
    resPacket.PlayerID = -1;

    // 1. 아이디가 존재하는지 확인
    if (AccountDB.find(requestID) != AccountDB.end())
    {
        // 2. 존재한다면 비밀번호 확인
        if (AccountDB[requestID] == requestPW)
        {
            resPacket.ResultCode = SUCCESS;

            // 세션에 고유 ID 부여 및 GameRoom 입장 처리
            session->_id = GlobalPlayerIDCounter++;
            resPacket.PlayerID = session->_id;

            std::cout << "Login Success: [" << requestID << "]" << std::endl;

            session->Send(&resPacket, sizeof(resPacket));
            
            GGameRoom.Enter(session);

            return;
        }
        else
        {
            resPacket.ResultCode = FAIL_WRONG_PW; // 비번 틀림
            std::cout << "Login Fail (Wrong PW): [" << requestID << "]" << std::endl;
        }
    }
    else
    {
        // 3. 아이디가 존재하지 않음 (유저 기획 포인트!)
        resPacket.ResultCode = FAIL_NOT_FOUND;
        std::cout << "Login Fail (Not Found): [" << requestID << "]" << std::endl;
    }

    // 결과 전송
    session->Send(&resPacket, sizeof(resPacket));
}

// [회원가입 요청 처리]
void PacketHandler::HandleRegisterReq(Session* session, PacketRegisterReq* packet)
{
    std::string requestID(packet->UserID);
    std::string requestPW(packet->Password);

    PacketLoginRes resPacket = {};
    resPacket.Size = sizeof(PacketLoginRes);
    resPacket.Id = S_TO_C_LOGIN_RES; // 응답은 로그인 응답 패킷 재사용

    // 중복 가입 체크 (그사이 누가 만들었을 수도 있으니)
    if (AccountDB.find(requestID) == AccountDB.end())
    {
        // DB에 추가
        AccountDB[requestID] = requestPW;
        resPacket.ResultCode = REGISTER_SUCCESS;
        std::cout << "New Account Created: [" << requestID << "]" << std::endl;

        PacketLoginReq dummyLoginReq = {};
        strncpy_s(dummyLoginReq.UserID, packet->UserID, sizeof(dummyLoginReq.UserID) - 1);
        strncpy_s(dummyLoginReq.Password, packet->Password, sizeof(dummyLoginReq.Password) - 1);

        HandleLoginReq(session, &dummyLoginReq);

        return;
    }
    else
    {
        resPacket.ResultCode = REGISTER_FAIL_DUP;
    }

    session->Send(&resPacket, sizeof(resPacket));
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

void PacketHandler::HandleChat(Session* session, PacketChat* packet)
{
    packet->SenderId = session->_id;

    packet->Id = S_TO_C_CHAT;

    std::cout << "[Chat] User " << session->_id << " : " << packet->Message << std::endl;
    GGameRoom.Broadcast(packet, packet->Size);
}
