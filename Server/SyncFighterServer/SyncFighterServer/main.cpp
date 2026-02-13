#pragma comment(lib, "ws2_32.lib")
#include "Session.h"
#include "GameRoom.h"
#include "PacketHandler.h"
#include <thread>
#include <vector>

// 전역 객체
HANDLE g_iocpHandle;

// [일꾼 스레드 함수]
void WorkerThread()
{
    while (true)
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0; // 우리가 세션 포인터로 쓸 예정
        OVERLAPPED* overlapped = nullptr;

        // 1. 큐에서 일감이 나올 때까지 대기 (GetQueuedCompletionStatus)
        // 일이 나오면 깨어남. (성공 시 TRUE 반환)
        BOOL result = GetQueuedCompletionStatus(
            g_iocpHandle,
            &bytesTransferred,
            &completionKey,
            &overlapped,
            INFINITE // 일이 없으면 무한 대기
        );

        // Session* session = (Session*)completionKey; // (주의: completionKey를 썼다면 이걸로 캐스팅)
        // 현재 코드에서는 overlapped 구조체 포인터로 세션을 찾고 있으니 아래 코드 유지
        OVERLAPPED_EX* exOverlapped = (OVERLAPPED_EX*)overlapped;
        Session* session = (Session*)exOverlapped->owner;

        // 2. 에러 처리 및 종료 확인 (수정된 부분 ★)
        if (result == FALSE || bytesTransferred == 0)
        {
            std::cout << "Client Disconnected: ID " << session->_id << std::endl;

            // ★ 방에서 내보내기 (이걸 안 해서 좀비가 됨)
            GGameRoom.Leave(session);

            // 소켓 닫기 및 메모리 해제
            // (Session 소멸자에서 closesocket 하도록 되어 있으므로 delete만 하면 됨)
            delete session;

            continue;
        }

        // 4. 작업 종류에 따라 처리
        if (exOverlapped->type == IO_TYPE::READ) // "데이터 받기 완료!"
        {
            PacketHandler::HandlePacket(session, session->_recvBuffer, bytesTransferred);

            session->Recv();
        }
    }
}

int main() {
    // 1. WinSock 초기화
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WinSock 초기화 실패!" << std::endl;
        return 0;
    }

    std::cout << "서버가 시작되었습니다." << std::endl;

    // --- 여기에 서버 로직이 들어갑니다 ---
    // 1. 소켓 생성 (전화기 구입)
    // AF_INET: IPv4 주소 체계 사용
    // SOCK_STREAM: TCP 연결 지향형 통신
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 주소 설정 (내 전화번호 설정)
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7777); // 사용할 포트 번호 (7777번)
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 내 PC의 아무 IP로든 접속 허용

    // 3. Bind (전화기에 번호 부여)
    if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Bind 실패!" << std::endl;
        return 0;
    }

    // 4. Listen (전화 대기 상태로 전환)
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Listen 실패!" << std::endl;
        return 0;
    }

    std::cout << "IOCP 서버 시작! (포트 7777)" << std::endl;

    // 1. IOCP 엔진 생성 (CreateIoCompletionPort)
    // 인자: 핸들(INVALID), 기존포트(NULL), 키(0), 스레드수(0=CPU개수만큼)
    g_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    // 2. 일꾼 스레드 생성 (CPU 개수만큼)
    // 여기서는 간단하게 2개만 만들어봅시다.
    std::vector<std::thread> workers;
    for (int i = 0; i < 2; ++i)
    {
        workers.emplace_back(WorkerThread);
    }

    int newSessionId = 1; // 유저에게 부여할 번호

    // 3. 접속 대기 루프 (Main Thread는 이제 '문지기' 역할만 함)
    while (true)
    {
        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);

        // 손님 입장 (Accept)
        SOCKET clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) continue;

        std::cout << "새로운 클라이언트 접속! ID: " << newSessionId << std::endl;

        // 4. 세션 객체 생성 (나중엔 메모리 풀 사용)
        Session* newSession = new Session();
        newSession->Init(clientSocket, newSessionId++);

        // 5. ★ 소켓과 IOCP 엔진 연결 (Bind IOCP) ★
        // "이 소켓(clientSocket)에서 발생하는 모든 일은 IOCP(g_iocpHandle)로 보내줘!"
        // CompletionKey로 'Session 포인터'를 넘겨줌 -> 나중에 일꾼이 이걸로 누구 건지 알 수 있음
        CreateIoCompletionPort((HANDLE)clientSocket, g_iocpHandle, (ULONG_PTR)newSession, 0);

        GGameRoom.Enter(newSession);

        newSession->Recv();
    }

    // (실제로는 여기까지 도달 안 함)
    for (auto& t : workers) t.join();
    CloseHandle(g_iocpHandle);
    WSACleanup();
    return 0;
}