#pragma once
#include <WinSock2.h>
#include <iostream>
#include "Protocol.h"

// 비동기 작업 식별용
enum class IO_TYPE { READ, WRITE, ACCEPT };

struct OVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;
	IO_TYPE type;
	void* owner; // 나중에 Session* 으로 캐스팅
};

class Session
{
public:
	Session() : _socket(INVALID_SOCKET), _id(-1) {}
	~Session() { if (_socket != INVALID_SOCKET) closesocket(_socket); }

	void Init(SOCKET s, int32_t id);
	void Send(void* packet, int32_t size);
	void Recv(); // 비동기 Recv 걸기

public:
	SOCKET _socket;
	int32_t _id;
	char _recvBuffer[1024];
	OVERLAPPED_EX _recvOverlapped;

public:
	// ★ 게임 데이터 추가
	// (원래는 Player 클래스를 따로 만드는 게 정석이지만, 일단 여기에 둡니다)
	float _x = 0, _y = 0, _z = 0, _yaw = 0; // 위치 기억
	int32_t _hp = 100;                      // 체력
	bool _isDead = false;                   // 사망 여부
};