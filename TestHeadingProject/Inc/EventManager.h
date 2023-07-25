#pragma once

#pragma pack(push, 1)
typedef Heading::SendStruct<10000, 12> PCK_CS_Enter;
typedef Heading::SendStruct<10001, 2> PCK_CS_Exit;
typedef Heading::SendStruct<10002, 102> PCK_CS_Chatting;
typedef Heading::SendStruct<10003, 114> PCK_CS_Wispering;
typedef Heading::SendStruct<10004, 8> PCK_SC_ReturnEnter;
typedef Heading::SendStruct<10004, 120> PCK_SC_OthersChatting;
#pragma pack(pop)

class EventManager
{
public:
	static void init();
	static EventManager* get();
	void Dispose();

	void AddBindPort( uint16_t _port );
	void NewSessionProcess();

	static void onAccept( SOCKET _sock );
	static void onSelect( WSAEVENT _sock );
	static void onRecv( WSAEVENT _sock );
	static void onSend( IN Heading::Header* _sendData );
	static void onEnter( WSAEVENT _sock, std::string _nickname );
	static void onExit( WSAEVENT _sock );
	static void onChatting( );
	static void onWispering( );

	static void Log( /*E_LOG_LEVEL _level,*/ std::string _log );
	static void logFlush( );

private:
	EventManager();
	~EventManager();

	static uint16_t m_sessionSize;

	static concurrency::concurrent_queue<std::string>				m_logQueue;

	Heading::CAccepter*												m_accepter;

	static EventManager*											m_instance;

	static CChatUser												m_userData;

	std::unordered_map<WSAEVENT, Heading::CClientSession*>			m_sessions; // Select 1Group == Zone
	static concurrency::concurrent_queue<Heading::CClientSession*>	m_acceptedSocket; // new Client

	Heading::Array_WSAEvent											m_wsaEvents; // Quick Remove Array
};

