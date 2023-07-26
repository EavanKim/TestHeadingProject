#pragma once

#pragma pack(push, 1)
typedef Heading::SendStruct<10000, 12> PCK_CS_Enter;
typedef Heading::SendStruct<10001, 2> PCK_CS_Exit;
typedef Heading::SendStruct<10002, 102> PCK_CS_Chatting;
typedef Heading::SendStruct<10003, 114> PCK_CS_Wispering;
typedef Heading::SendStruct<10004, 8> PCK_CS_RequestPrevious;
typedef Heading::SendStruct<10005, 8> PCK_SC_ReturnEnter;
typedef Heading::SendStruct<10006, 120> PCK_SC_OthersChatting;
#pragma pack(pop)

enum E_PCK_TYPE
{
	PCK_CS_ENTER = 10000,
	PCK_CS_EXIT,
	PCK_CS_CHATTING,
	PCK_CS_WISPERING,
	PCK_CS_REQUESTPREVIOUS,
	PCK_SC_RETURNENTER,
	PCK_SC_OTHERSCHATTING,
	PCK_CS_MAX
};

class EventManager
{
public:
	static void init();
	static EventManager* get();
	void Dispose();

	static void onAccept( SOCKET _sock );

	void onSelect( DWORD _eventIndex );
	void onRecv( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData );
	void onSend( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData );
	void onEnter( IN Heading::CClientSession* _sessionInfo, IN PCK_CS_Enter* _recvData );
	void onExit( IN Heading::CClientSession* _sessionInfo );
	void onChatting( IN Heading::CClientSession* _sessionInfo, IN PCK_CS_Chatting* _sendData );
	void onWispering( IN Heading::CClientSession* _sessionInfo, IN PCK_CS_Wispering* _sendData );
	void onRequestPrevious(IN Heading::CClientSession* _sessionInfo, IN PCK_CS_RequestPrevious* _sendData);

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
};

