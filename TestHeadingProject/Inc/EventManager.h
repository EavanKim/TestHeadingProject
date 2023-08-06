#pragma once

enum E_EventManager_State
{
	EventManager_Server,
	EventManager_Client,
	EventManager_MAX
};

enum E_PCK_CS_TYPE
{
	PCK_CS_Shutdown = 1,
	PCK_CS_Pong,
	PCK_CS_ENTER,
	PCK_CS_EXIT,
	PCK_CS_CHATTING,
	PCK_CS_WISPERING,
	PCK_CS_REQUESTPREVIOUS,
	PCK_CS_MAX
};

enum E_PCK_SC_TYPE
{
	PCK_SC_SessionKey = 1,
	PCK_SC_Ping,
	PCK_SC_WISPERING,
	PCK_SC_RETURNENTER,
	PCK_SC_OTHERSCHATTING,
	PCK_SC_REQUESTRESET,
	PCK_SC_MAX
};

// 이벤트 관리자 사이즈를 벗어나서 재조정 필요.
// 전송 수신 기능과 세션 관리 기능은 서버엡으로 이전
class EventManager
{
public:
	static	void			init					( E_PCK_CS_TYPE _type );
	static	EventManager*	get						( );
			void			Destroy					( );
			void			Dispose					( );

	static	void			onAccept				( SOCKET _sock );
			void			SetAcceptSession		( );
			void			FlushSend				( );

			void			onSelect				( DWORD _eventIndex );
			void			onRecv					( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
			void			onSend					( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );

			void			Remove_Event			( WSAEVENT _key );
			void			Recreate_EventInfo		( );

			void			Log						( /*E_LOG_LEVEL _level,*/ std::string _log );
			void			logFlush				( );

			uint8_t			GetEventSize			( );
			WSAEVENT*		GetEventArray			( );

private:
							EventManager			( E_PCK_CS_TYPE _type );
							~EventManager			( );

			void			InitializeClient		( );
			void			InitializeServer		( );

	// Server Recv Event
	static	void			onShutdown				( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onPong					( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onEnter					( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onExit					( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onChatting				( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );
	static	void			onWispering				( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );
	static	void			onRequestPrevious		( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );
	static	void			onRequestReset			( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );

	// Client Recv Event
	static	void			onSessionKey			( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onPing					( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onWispering				( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onReturnEnter			( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onOthersChatting		( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static	void			onRequestReset			( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );

	static	void			onNonDefinedCallback	( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );

	Heading::CPacketHandler											m_handler;

	static EventManager*											m_instance;
	
	static uint16_t m_sessionSize;

	concurrency::concurrent_queue<std::string>						m_logQueue;

	CChatUser														m_userData;

	std::unordered_map<WSAEVENT, Heading::CClientSession*>			m_sessions;			// Select 1Group == Zone
	static concurrency::concurrent_queue<Heading::CClientSession*>	m_acceptedSocket;	// new Client

	Heading::Array_WSAEvent											m_wsaEvents;		// Quick Remove Array
};

