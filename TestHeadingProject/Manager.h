#pragma once

namespace Heading
{
	namespace SimpleServerKit
	{
		typedef std::unordered_map<WSAEVENT, CEventBaseSession*> sessionMap;
		typedef std::unordered_map<uint64_t, CSelecter*> selectMap;

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

		class Manager
		{
		public:
			static void Init(E_LOG_LEVEL _logLevel, uint64_t _selectThreadCount);
			static Manager* Get();
			void Dispose( );

			void Start( uint16_t _port );
			void End( uint16_t _port );

			void Update();

			static int Accept( void* _ptr );

			void server_log(E_LOG_LEVEL _level, std::string _log);
			void server_log_flush();

			bool try_set_new_session(CreatedSocketInfo& _socket);

		private:
			Manager(E_LOG_LEVEL _logLevel, uint64_t _selectThreadCount);
			~Manager();

			static Manager* m_instance;

			volatile LONG64 m_managerWork = 0;

			concurrency::concurrent_queue<SessionData*> m_recvQueue;
			concurrency::concurrent_queue<SessionData*> m_sendQueue;

			concurrency::concurrent_queue<std::string> m_logQueue;

			concurrency::concurrent_queue<CreatedSocketInfo> m_newSocketQueue;

			std::vector<CPacketHandler*> m_handlers = {};

			std::vector<std::thread*> m_threads;

			E_LOG_LEVEL m_logLevel = E_LOG_LEVEL::E_LOG_LEVEL_NONE;
			ManagerState_CountOf m_state_count = {};
			WSADATA m_data = {};
			sessionMap m_sessions = {};
			CAccept_Mgr m_login = {};
			selectMap m_zone = {};
		};
	}
}

