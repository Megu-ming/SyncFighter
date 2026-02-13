#include "Session.h"
#include <iostream>

void Session::Init(SOCKET s, int32_t id)
{
	_socket = s;
	_id = id;
	_recvOverlapped.type = IO_TYPE::READ;
	_recvOverlapped.owner = this;
}

void Session::Send(void* packet, int32_t size)
{
	// 지금은 간단하게 Blocking Send 사용
	// (나중에 WSASend를 이용한 비동기 큐 방식으로 고도화 가능)
	int sent = send(_socket, (char*)packet, size, 0);
	if (sent == SOCKET_ERROR)
	{
		std::cout << "Send Failed" << std::endl;
	}
}

void Session::Recv()
{
	DWORD flags = 0;
	WSABUF wsaBuf;
	wsaBuf.buf = _recvBuffer;
	wsaBuf.len = sizeof(_recvBuffer);

	// 구조체 초기화 (중요)
	ZeroMemory(&_recvOverlapped.overlapped, sizeof(WSAOVERLAPPED));

	// IOCP에 Recv 작업 걸기
	int ret = WSARecv(_socket, &wsaBuf, 1, NULL, &flags, &_recvOverlapped.overlapped, NULL);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			std::cout << "Recv Failed: " << err << std::endl;
			// 여기서 연결 끊김 처리 등을 할 수 있음
		}
	}
}