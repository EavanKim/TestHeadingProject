#pragma once

typedef std::unordered_map<WSAEVENT, Heading::CEventBaseSession*> sessionMap;
typedef std::unordered_map<uint64_t, Heading::CSelecter*> selectMap;
typedef concurrency::concurrent_vector<Heading::CSelecter*> selectList;

enum E_LOG_LEVEL
{
	E_LOG_LEVEL_NONE,
	E_LOG_LEVEL_RELEASE,
	E_LOG_LEVEL_DEBUG,
	E_LOG_LEVEL_MAX
};

struct ManagerState_CountOf
{
	uint64_t m_acceptThread = 0;
	uint64_t m_selectThread = 0;
	uint64_t m_maximumSession = 0;
	uint64_t m_currentSession = 0;
};

struct AcceptCommand
{
	bool CreateCommand = false;
	uint16_t Port = 0;
};

class Manager
{
public:
	static void Init( E_LOG_LEVEL _logLevel );
	static Manager* Get( );
	void Dispose( );

	void ChattingStartUp();

	void Update( );

	static int Accept( void* _ptr );

	bool get_newAcceptPort( uint16_t& _port );

	void server_log( E_LOG_LEVEL _level, std::string _log );
	void server_log_flush( );

	void add_accepted_socket( Heading::CreatedSocketInfo& _socket );

	bool try_set_new_session( Heading::CreatedSocketInfo& _socket );

private:
	Manager( E_LOG_LEVEL _logLevel );
	~Manager( );

	static Manager* m_instance;

	volatile LONG64 m_managerWork = 0;

	// 내게 필요한 비동기 큐 종류.
	// 1. Accepter 에서 처리할 정보
	//		1-1 새로운 연결을 위한 포트번호
	//		1-2 더이상 처리하지 않을 포트번호
	//		1-3 새로 연결된 소켓정보
	//			1-3-1 연결이 들어온 포트정보 - 용도 결정
	// 2. Session 관련 정보
	//		2-1 메인 로직에서 처리할 수신 데이터
	//		2-2 메인 로직에서 요청한 송신 데이터
	//		2-3 소켓의 각종 상태 설정
	//		2-4 소켓의 각종 상태 확인
	// 3. 상태에 대한 시각적 정보
	//		3-1 모든 스레드에서 발생한 로그정보
	concurrency::concurrent_queue<Heading::SessionData*> m_recvQueue;
	concurrency::concurrent_queue<Heading::SessionData*> m_sendQueue;

	// Accept Port
	concurrency::concurrent_queue<AcceptCommand> m_portQueue;

	// log
	concurrency::concurrent_queue<std::string> m_logQueue;

	// new Socket
	concurrency::concurrent_queue<Heading::CreatedSocketInfo> m_newSocketQueue;

	// handler 
	std::vector<Heading::CPacketHandler*> m_handlers = {};

	// 목표로 하는 Thread를 찾아서 지울 방법에 대한 고민
	std::unordered_map<uint16_t, std::thread*> m_acceptThreads = {};
	std::unordered_map<WSAEVENT, std::thread*> m_selectThreads = {};

	// 이건 인덱스를 키로 사용해서 자신이 요청한 일에 대해서 인덱스로 접근하면서 체크하기.
	std::vector<std::thread*> m_backgroundThreads = {};

	std::atomic<bool> m_processLive;

	E_LOG_LEVEL m_logLevel = E_LOG_LEVEL::E_LOG_LEVEL_NONE;
	ManagerState_CountOf m_state_count = {};
	Heading::CAccept_Mgr m_login = {};
	sessionMap m_sessions = {};
	selectList m_zone;
	WSADATA m_data = {};
};

