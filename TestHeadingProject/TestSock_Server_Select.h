#pragma once

//이 객체 하나당 1session.
// 작업자가 알아보기 쉬운 이름입니다.
class CSession;
class TestSock_Server_Select
{
public:
	TestSock_Server_Select(uint64_t _port);
	~TestSock_Server_Select();

	int CreateInitializeData();
	void ListenBind();
	void Select();
	void CreateNewSession();
	void Do( SOCKET _index );
	void Update();

	void Dispose();

private:
	SOCKET m_bind = INVALID_SOCKET;
	uint64_t m_port = 0;
	addrinfo* m_info = nullptr;
	WSADATA m_data = {};
	sockaddr_in m_listenIn = {};
	sockaddr_in m_acceptIn = {};
	fd_set m_readfds = {};
	std::unordered_map<SOCKET, CSession*> m_sessionMap;
	PrintLog m_log;
};

