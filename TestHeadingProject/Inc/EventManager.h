#pragma once

enum E_PCK_TYPE
{
	PCK_SessionKey,
	PCK_Shutdown,
	PCK_Ping,
	PCK_Pong,
	PCK_CS_ENTER,
	PCK_CS_EXIT,
	PCK_CS_CHATTING,
	PCK_CS_WISPERING,
	PCK_SC_WISPERING,
	PCK_CS_REQUESTPREVIOUS,
	PCK_SC_RETURNENTER,
	PCK_SC_OTHERSCHATTING,
	PCK_SC_REQUESTRESET,
	PCK_CS_MAX
};

class EventManager
{
public:
	static void init( );
	static EventManager* get( );
	void Destroy( );
	void Dispose( );

	static void onAccept( SOCKET _sock );
	void SetAcceptSession( );
	void FlushSend( );

	void onSelect( DWORD _eventIndex );
	void onRecv( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	void onSend( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );

	static void onPong( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static void onEnter( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static void onExit( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	static void onChatting( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );
	static void onWispering( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );
	static void onRequestPrevious( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );

	static void onNonDefinedCallback( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );

	void Remove_Event( WSAEVENT _key );
	void Recreate_EventInfo( );

	void Log( /*E_LOG_LEVEL _level,*/ std::string _log );
	void logFlush( );

	uint8_t GetEventSize( );
	WSAEVENT* GetEventArray( );

private:
	EventManager( );
	~EventManager( );

	Heading::CPacketHandler											m_handler;

	static EventManager*											m_instance;
	
	static uint16_t m_sessionSize;

	concurrency::concurrent_queue<std::string>						m_logQueue;

	CChatUser														m_userData;

	std::unordered_map<WSAEVENT, Heading::CClientSession*>			m_sessions;			// Select 1Group == Zone
	static concurrency::concurrent_queue<Heading::CClientSession*>	m_acceptedSocket;	// new Client

	Heading::Array_WSAEvent											m_wsaEvents;		// Quick Remove Array
};

