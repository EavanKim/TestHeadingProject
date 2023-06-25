#pragma once
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>

#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>

//이 객체 하나당 1session.
// 작업자가 알아보기 쉬운 이름입니다.
class CSession;
class TestSock_Server
{
public:
	TestSock_Server( uint64_t _port );
	~TestSock_Server();

	void CreateInitializeData();
	void Binding();
	CSession* Wating();
	void CloseSession(CSession* _close);

private:
	WSADATA m_data = {};
	uint64_t m_port = 0;
	addrinfo* m_info = nullptr;
	sockaddr_in m_service = {};
	SOCKET m_listenSock = INVALID_SOCKET;
	std::vector<CSession*> m_sessionList = {};
};

