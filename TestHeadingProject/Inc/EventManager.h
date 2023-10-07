#pragma once


//typedef SendStruct<1, 1> PCK_SessionKey;
//typedef SendStruct<2, 1> PCK_Shutdown;
//typedef SendStruct<3, 1> PCK_Ping;
//typedef SendStruct<4, 1> PCK_Pong;
//typedef SendStruct<10000, 12> PCK_CS_Enter;
//typedef SendStruct<10001, 2> PCK_CS_Exit;
//typedef SendStruct<10002, 100> PCK_CS_Chatting;
//typedef SendStruct<10003, 112> PCK_CS_Wispering;
//typedef SendStruct<10004, 112> PCK_SC_Wispering;
//typedef SendStruct<10005, 8> PCK_CS_RequestPrevious;
//typedef SendStruct<10006, 8> PCK_SC_ReturnEnter;
//typedef SendStruct<10007, 120> PCK_SC_OthersChatting;
//typedef SendStruct<10008, 8> PCK_SC_RequestReset;
enum E_PCK_TYPE
{
	PCK_SessionKey = 1,
	PCK_Shutdown,
	PCK_Ping,
	PCK_Pong,
	PCK_CS_ENTER = 10000,
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
	static void init();
	static EventManager* get();
	void Destroy();
	void Dispose();

	static void onAccept( SOCKET _sock );
	static void onAccept_v2( SOCKET _sock );
	void SetAcceptSession();
	void FlushSend();

	void onSelect( DWORD _eventIndex );
	void onRecv( IN Heading::CClientSession* _sessionInfo, IN Heading::packetHeader_t* _recvData );
	void onSend( IN Heading::CClientSession* _sessionInfo, IN Heading::packetHeader_t* _sendData );
	void onEnter( IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_Enter* _recvData );
	void onExit( IN Heading::CClientSession* _sessionInfo );
	void onChatting( IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_Chatting* _sendData );
	void onWispering( IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_Wispering* _sendData );
	void onRequestPrevious(IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_RequestPrevious* _sendData);

	void Remove_Event( WSAEVENT _key );
	void Recreate_EventInfo();

	void Log( /*E_LOG_LEVEL _level,*/ std::string _log );
	void logFlush( );

	uint8_t GetEventSize();
	WSAEVENT* GetEventArray();

private:
	EventManager();
	~EventManager();

	static EventManager*											m_instance;
	
	static uint16_t m_sessionSize;

	concurrency::concurrent_queue<std::string>						m_logQueue;

	CChatUser														m_userData;

	std::unordered_map<WSAEVENT, Heading::CClientSession*>			m_sessions;			// Select 1Group == Zone
	static concurrency::concurrent_queue<Heading::CClientSession*>	m_acceptedSocket;	// new Client

	Heading::Array_WSAEvent											m_wsaEvents;		// Quick Remove Array

	static concurrency::concurrent_queue<Heading::CSimpleSocket*>	m_newAccessList;
};

