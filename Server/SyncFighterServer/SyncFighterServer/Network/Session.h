#pragma once
#include <WinSock2.h>
#include <iostream>
#include "Protocol.h"

class Player;

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
	// 게임 데이터 추가
	Player* _player;
};