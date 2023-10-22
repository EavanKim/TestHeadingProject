//#pragma once
//
//enum class Server_State
//{
//	INITIALIZE,
//	STARTUP,
//	PRE,
//	RUN,
//	POST,
//	END,
//	MAX
//};
//
//struct SessionInfo
//{
//	OVERLAPPED overlapped_ = {};
//	SOCKET sock_ = INVALID_SOCKET;
//	uint64_t recvBytes_ = 0;
//	uint64_t sendBytes_ = 0;
//	WSABUF wsaBuf_ = {};
//	Heading::CSessionStorage storage_;
//};
//
//class CServer_App_Completion
//{
//public:
//	static void Initialize();
//	static CServer_App_Completion* get();
//	void dispose();
//
//	void Prepare();
//	void Process_Accept();
//
//private:
//	CServer_App_Completion();
//	~CServer_App_Completion();
//
//	Server_State m_state = Server_State::INITIALIZE;
//
//	Heading::CAccepter* m_accepter = nullptr;
//	HANDLE m_completionHandle = INVALID_HANDLE_VALUE;
//	std::vector<HANDLE> m_workerThread = {};
//
//	static CServer_App_Completion* m_instance;
//};
//
