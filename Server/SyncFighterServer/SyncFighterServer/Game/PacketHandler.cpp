#include "PacketHandler.h"
#include "GameRoom.h" // GameRoom을 알아야 브로드캐스팅을 하니까
#include <cmath>
#include <unordered_map>
#include <string>
#include <cstring>
#include <vector>
std::vector<Session*> MatchQueue;

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
    case C_TO_S_MATCH_REQ:
        HandleMatchReq(session, (PacketMatchReq*)buffer);
        break;
    case C_TO_S_MATCH_CANCEL_REQ:
        HandleMatchCancelReq(session, (PacketMatchCancelReq*)buffer);
        break;
    case C_TO_S_ENTER_GAME_REQ:
        HandleEnterGameReq(session, (PacketEnterGameReq*)buffer);
        break;
	case C_TO_S_PLAYER_MOVE:
		HandlePlayerMove(session, (PacketPlayerMove*)buffer);
		break;
	case C_TO_S_PLAYER_ATTACK:
		HandlePlayerAttack(session, (PacketPlayerAttack*)buffer);
		break;
    case C_TO_S_PLAYER_SKILL:
        HandlePlayerSkill(session, (PacketPlayerSkill*)buffer);
        break;
    case C_TO_S_HIT_REQ:
        HandleHitReq(session, (PacketHitReq*)buffer);
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
        // 3. 아이디가 존재하지 않음
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

void PacketHandler::HandleMatchReq(Session* session, PacketMatchReq* packet)
{
    session->_classType = packet->ClassType;

    // 1. 대기열에 유저 추가
    MatchQueue.push_back(session);
    std::cout << "[Matchmaker] User " << session->_id << " 이 매칭 큐에 등록되었습니다. (현재 대기자: " << MatchQueue.size() << "명)" << std::endl;

    // 2. 2명이 모였는지 확인!
    if (MatchQueue.size() >= 2)
    {
        Session* player1 = MatchQueue[0];
        Session* player2 = MatchQueue[1];

        // 큐에서 두 명 제거
        MatchQueue.erase(MatchQueue.begin(), MatchQueue.begin() + 2);

        std::cout << "[Matchmaker] 매칭 성사! User " << player1->_id << " & User " << player2->_id << std::endl;

        // 3. 두 명 모두에게 "매칭 성공! 맵 넘어가라!" 패킷 전송
        PacketMatchSuccess successPkt;
        successPkt.Size = sizeof(PacketMatchSuccess);
        successPkt.Id = S_TO_C_MATCH_SUCCESS;

        player1->Send(&successPkt, sizeof(successPkt));
        player2->Send(&successPkt, sizeof(successPkt));
    }
}

void PacketHandler::HandleMatchCancelReq(Session* session, PacketMatchCancelReq* packet)
{
    for (auto it = MatchQueue.begin(); it != MatchQueue.end(); ++it)
    {
        if (*it == session)
        {
            MatchQueue.erase(it);
            std::cout << "[Matchmaker] User " << session->_id << " 매칭 취소. (남은 대기자: " << MatchQueue.size() << "명)" << std::endl;
            break;
        }
    }
}

void PacketHandler::HandleEnterGameReq(Session* session, PacketEnterGameReq* packet)
{
    session->_classType = packet->ClassType;

    std::cout << "[User " << session->_id << "] Enter Game! Class: " << session->_classType << std::endl;

    // 직업 선택 했으니 게임룸 입장
    GGameRoom.Enter(session);
}

void PacketHandler::HandlePlayerMove(Session* session, PacketPlayerMove* packet)
{
	session->_x = packet->X;
	session->_y = packet->Y;
	session->_z = packet->Z;
	session->_yaw = packet->Yaw;

	// 패킷 검증 및 수정
	packet->PlayerID = session->_id;
    packet->ClassType = session->_classType;

	// 방에 있는 사람들에게 전송
	GGameRoom.Broadcast(packet, packet->Size, session);
}

void PacketHandler::HandlePlayerAttack(Session* session, PacketPlayerAttack* packet)
{
	packet->PlayerID = session->_id;
	std::cout << "[User " << session->_id << "] Attack!" << std::endl;

	// 방에 뿌리기
	GGameRoom.Broadcast(packet, packet->Size, session);

	//GGameRoom.HandleAttack(session);
}

void PacketHandler::HandlePlayerSkill(Session* session, PacketPlayerSkill* packet)
{
    packet->PlayerID = session->_id;
    std::cout << "[User " << session->_id << "] " << packet->SkillIndex << " Skill!" << std::endl;

    GGameRoom.Broadcast(packet, packet->Size, session);
}

void PacketHandler::HandleHitReq(Session* session, PacketHitReq* packet)
{
    // 서버에서 관리하는 유저의 체력을 깎아야 하지만,
    Session* VictimSession = GGameRoom.FindSession(packet->VictimID);

    if (VictimSession != nullptr) 
    {
        if (VictimSession == session) return;
        if (VictimSession->_isDead) return;

        // 2. 체력 차감
        VictimSession->_hp -= packet->Damage;
        if (VictimSession->_hp <= 0)
        {
            VictimSession->_hp = 0;
            VictimSession->_isDead = true;

            VictimSession->_deaths++;
            session->_kills++;

            int32_t victimId = VictimSession->_id;

            std::thread([victimId]() {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                GGameRoom.Respawn(victimId); // 5초 뒤 부활!
                }).detach();
        }

        // 3. 데미지 결과 패킷 세팅
        PacketDamage ResPkt = {};
        ResPkt.Size = sizeof(PacketDamage);
        ResPkt.Id = S_TO_C_DAMAGE;

        ResPkt.AttackerID = session->_id;       // 때린 사람ㅍㅍ
        ResPkt.VictimID = packet->VictimID;     // 맞은 사람
        ResPkt.DamageAmount = packet->Damage;   // 들어간 데미지
        ResPkt.RemainingHP = VictimSession->_hp; // 진짜 남은 체력!

        std::cout << "[Hit] 유저 " << session->_id << " -> 유저 " << packet->VictimID
            << " 타격! (데미지: " << packet->Damage
            << " / 남은 체력: " << VictimSession->_hp << ")" << std::endl;

        // 4. 방에 있는 모두에게 데미지 결과 브로드캐스팅
        GGameRoom.Broadcast(&ResPkt, ResPkt.Size);
    }
}

void PacketHandler::HandleChat(Session* session, PacketChat* packet)
{
    packet->SenderId = session->_id;

    packet->Id = S_TO_C_CHAT;

    std::cout << "[Chat] User " << session->_id << " : " << packet->Message << std::endl;
    GGameRoom.Broadcast(packet, packet->Size);
}
