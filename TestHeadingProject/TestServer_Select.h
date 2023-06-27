#pragma once

class TestServer_Select
{
public:
	TestServer_Select( uint64_t _port, ENUM_SESSION_TYPE _type = ENUM_SESSION_SYNC );
	~TestServer_Select();

	int CreateInitializeData();
	void ListenBind();
	void Select();
	void CreateNewSession();
	void Do( SOCKET _index );
	void Update();

	void Dispose();

private:
	ENUM_SESSION_TYPE m_sessionType = ENUM_SESSION_SYNC;
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

