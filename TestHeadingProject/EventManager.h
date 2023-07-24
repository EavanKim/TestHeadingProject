#pragma once

//===============================================================================================
// 
// 서버 상태의 핵심은 무슨 일을 하고 있느냐이지, 어떤 대상에게 하느냐는 다음 문제.
// 서버가 현재 행하는 일을 기준으로 일을 풀어냅니다.
//		-> 어차피 특정 유저는 지정된 동작만 한다, 이런게 아니라 하나의 서비스로 들어온 사람들에겐 모두 같은 동작을 해서 돌려줘야 하기 때문.
//			-> 다만 각 연결 별 상태값이 있고 이 상태값의 차이에 따라 같은 서비스를 통해 다른 결과값을 이끌어내는 것
//			-> 메인 페이지를 보는 유저와 방명록을 보는 유저는 서로 같은 홈페이지를 사용하고 같은 서비스를 쓰지만, 서로의 위치가 메인페이지와 방명록으로 다르기 때문에 다른 화면을 보는 것.
//			-> 동작에 관한 기본 추상화 방향
//				-> 클라이언트에서는 다른 기능 별(그리기를 하는 Graphics API와 소리를 내는 Sound API는 모두 Tick 혹은 Update라는 같은 단위에서 새로운 상태로 진행) 추상화 끝에 같은 동작까지 올라온다면
//				-> 서버는 같은 추상화에서 다른 기능(서버는 데이터를 받고, 처리하고, 내보내는 상태가 존재하고 모든 동작은 이 세 처리에 기반하여 상세한 세부 구현)으로 분화
//		-> 서버의 서비스가 유저별로 서로 다른게 훨씬 문제 // 돈을 낸 기준 등으로 유저를 그룹으로 나누면 해당 그룹은 모두 같은 서비스를 제공하기 때문
//		-> 서버식 생각의 기초로 봐야 할 것.
//		-> 이벤트 드리븐이라 해도 엔진에서 사용하는 이벤트 드리븐과의 개념이 다른부분에 주의(엔진 -> 특정 상황에 객체별 특수화를 통한 이벤트 처리 / 서버 -> 특정 상황에 처리해야할 데이터의 transactional한 처리 : 이 처리를 객체별로 나누어서 동일한 동작처럼 감싸는 것)
//		-> 즉, 엔진은 개별 객체 배열을 개별 관리하고 서버는 개별 동작 상황 별 데이터를 처리
// 
//===============================================================================================

// 이벤트를 처리할 때 이벤트 주체도 중요.



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


	// 서버가 ~~~를 한다

	// 서버가 새로운 연결을 한다
	static void onAccept( SOCKET _sock );

	// 서버가 네트워크 io 이벤트를 처리한다
	static void onSelect( );

	// 서버가 데이터를 받는다.
	static void onRecv( );

	// 서버가 데이터를 보낸다
	static void onSend( IN Heading::Header* _sendData );

	// 패킷으로 ~~~를 한다	
	// Accept된 소켓에서 들어온 첫 패킷 정보를 통해 유저를 특정한다
	static void onEnter( WSAEVENT _sock );

	// 통신중인 소켓에 문제가 생겼거나 끊기 요청이 들어온 부분을 처리한다.
	static void onExit( WSAEVENT _sock );

	// 채팅 통신에 대해 일관 처리한다.
	static void onChatting( );

	// 특정 유저에게만 보내는 귓속말 기능.
	static void onWispering( );

private:
	EventManager();
	~EventManager();

	static uint16_t m_sessionSize;

	static EventManager* m_instance;

	// 검색용 데이터
	static concurrency::concurrent_unordered_map<std::string, WSAEVENT>		m_userTable;

	// 검색 및 저장용 데이터
	std::unordered_map<WSAEVENT, Heading::CClientSession*>					m_sessions;

	// 새로운 연결용 데이터
	static concurrency::concurrent_queue<Heading::CClientSession*>			m_acceptedSocket;

	// 특정 동작용 데이터(Wait)
	Heading::Array_WSAEvent													m_wsaEvents;
};

