#include "psudoPCH.h"

EventManager*											EventManager::m_instance		= nullptr;
uint16_t												EventManager::m_sessionSize		= 0;
concurrency::concurrent_queue<Heading::CClientSession*>	EventManager::m_acceptedSocket;

void EventManager::init( )
{
	if( nullptr == m_instance )
		m_instance = new EventManager();
}

EventManager* EventManager::get( )
{
	return m_instance;
}

void EventManager::Destroy( )
{
	Dispose();
	delete m_instance;
}

void EventManager::Dispose( )
{
	// 소유권이 없는 객체들 선행 비우기
	m_userData.clear();
	// m_wsaEvents - 이건 그냥 무시해버리기.

	// 큐 컨테이너도 비워줄 겸 pop 처리합니다.
	while( !m_acceptedSocket.empty( ) )
	{
		Heading::CClientSession* session = nullptr;
		if( m_acceptedSocket.try_pop( session ) )
		{
			delete session;
		}
	}

	for( auto& iter : m_sessions )
	{
		WSAEVENT currentEvent = iter.first;
		Heading::CClientSession* currentSession = iter.second;

		WSACloseEvent( currentEvent );
		delete currentSession;
	}

	// 마지막 처리 끝날 때 까지 나온 모든 로그를 송출
	logFlush();
}

void EventManager::onAccept( SOCKET _sock )
{
	EventManager::get()->Log( Heading::formatf( "onAccept : %lld", _sock ) );
	// 지금은 단순히 채팅 세션만을 만들어서 저장할 것

	if( WSA_MAXIMUM_WAIT_EVENTS > m_sessionSize )
	{
		m_acceptedSocket.push( new CChatSession( _sock ) );
	}
	else
	{
		closesocket( _sock );
	}
}

void EventManager::SetAcceptSession( )
{
	if( !m_acceptedSocket.empty( ) )
	{
		Heading::CClientSession* ptr = nullptr;
		if( m_acceptedSocket.try_pop( ptr ) )
		{
			ptr->CreateAndSetEvent( FD_READ | FD_WRITE );
			WSAEVENT newEvent = ptr->Get_Event( );
			if( INVALID_HANDLE_VALUE != newEvent )
			{
				m_wsaEvents.add( newEvent );
				m_sessions.insert( std::make_pair( newEvent, ptr ) );
			}
			else
			{
				delete ptr;
			}
		}
	}
}

void EventManager::FlushSend( )
{
	for( auto iter : m_sessions )
	{
		Heading::CClientSession* curSession = iter.second;

		curSession->SendData();
	}
}

void EventManager::onSelect( DWORD _eventIndex )
{
	// WSAWaitForMultipleEvents Result
	WSAEVENT currentEvent = m_wsaEvents[ _eventIndex ];
	auto iter = m_sessions.find(currentEvent);
	if( m_sessions.end( ) != iter )
	{
		Heading::CClientSession* current = iter->second;
		WSANETWORKEVENTS NetworkEvents;
		int retval = WSAEnumNetworkEvents( current->Get_Sock(), current->Get_Event(), &NetworkEvents );
		if( retval == SOCKET_ERROR )
		{
			Log( Heading::formatf("EventManager::onSelect Fail. EnumNetworkEvents returned SOCKET_ERROR %i", _eventIndex) );
			return;
		}

		if( NetworkEvents.lNetworkEvents & FD_READ )
		{
			if( NetworkEvents.lNetworkEvents & FD_READ
				&& NetworkEvents.iErrorCode[ FD_READ_BIT ] != 0 )
			{
				return;
			}
			Heading::packetBuff buff; // local buffer니까 사라질거라고 보고 클리어 하지 않습니다.
			current->RecvData( OUT buff );

			while( !buff.empty( ) )
			{
				Heading::Header* packet = buff.front();
				if( nullptr != packet )
				{
					if( current->CheckLive( ) ) // 받는 중 세션이 죽은게 확인되면 큐만 날리게 Recv 이벤트 처리를 안합니다.
					{
						onRecv( current, packet );
					}

					delete packet;

					buff.pop();
				}
				else
				{
					// null 은 크래시 사유라는 공유를 따라 크래시시킵니다.
					throw std::exception( "Receive Null Crash!!!" );
				}
			}

			if( !current->CheckLive( ) )
			{
				Remove_Event( currentEvent );
				return;
			}
		}

		if( NetworkEvents.lNetworkEvents & FD_WRITE )
		{
			if( NetworkEvents.lNetworkEvents & FD_WRITE
				&& NetworkEvents.iErrorCode[ FD_WRITE_BIT ] != 0 )
			{
				return;
			}
			current->onEventSend();
		}

		if( NetworkEvents.lNetworkEvents & FD_CONNECT )
		{
			if( NetworkEvents.lNetworkEvents & FD_CONNECT
				&& NetworkEvents.iErrorCode[ FD_CONNECT_BIT ] != 0 )
			{
				return;
			}
			current->onEventSend();
		}

		if( NetworkEvents.lNetworkEvents & FD_CLOSE )
		{
			Remove_Event( currentEvent );
			delete current;
			return;
		}
	}
}

void EventManager::onRecv( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData )
{
	m_handler.Do_Process(_sessionInfo, _recvData);
}

// 여기는 나중에 onSend 들어갔을 때 데이터를 암호화 하는데 써야할지 고민 해 보기
// 이 타이밍에 할 일이 무엇이 있을까...?
void EventManager::onSend( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{

}

// 접속 한 유저의 계정정보 설정
void EventManager::onEnter( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData )
{
	EventManager* mgr = EventManager::get();

	if( nullptr != mgr )
	{
		Heading::PCK_CS_Enter* parse = static_cast< Heading::PCK_CS_Enter* >( _recvData );
		mgr->Log( Heading::formatf( "Enter : %s", parse->buffer ) );
		mgr->m_userData.Add( _sessionInfo->Get_Event( ), parse->buffer );
	}
}

// 유저 탈출
void EventManager::onExit( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData )
{
	EventManager* mgr = EventManager::get();

	if( nullptr != mgr )
	{
		WSAEVENT targetEvent = _sessionInfo->Get_Event( );
		std::string nickName = mgr->m_userData.find( targetEvent );
		mgr->Log( Heading::formatf( "Exit : %s", nickName ) );
		mgr->Remove_Event( targetEvent );
		delete _sessionInfo;
	}
}

// 채팅 받음
// 바로 다른 유저 세션 전송큐에 대기시켜놓기
void EventManager::onChatting( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{
	EventManager* mgr = EventManager::get();

	if( nullptr != mgr )
	{
		Heading::PCK_CS_Chatting* parse = static_cast< Heading::PCK_CS_Chatting* >( _sendData );
		mgr->Log( Heading::formatf( "%s : %s", mgr->m_userData.find( _sessionInfo->Get_Event( ) ).c_str( ), parse->buffer ) );
		for( auto iter = mgr->m_sessions.begin( ); mgr->m_sessions.end( ) != iter; ++iter )
		{
			Heading::CClientSession* currentSession = iter->second;

			currentSession->enqueueSend( _sendData );
		}
	}
}

// 귓속말 받음
// 바로 대상 유저 세션 전송큐에 대기시켜놓기
void EventManager::onWispering( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{
	EventManager* mgr = EventManager::get();
	
	if( nullptr != mgr )
	{
		Heading::PCK_CS_Wispering* parse = static_cast<Heading::PCK_CS_Wispering*>(_sendData);
		std::string name( 13, '\0' );// 닉네임 풀 사이즈 12
		memcpy_s( name.data( ), 12, parse->buffer, 12 );
		std::string chat( 101, '\0' ); // 채팅 풀 사이즈 100
		memcpy_s( chat.data( ), 100, ( ( char* ) parse->buffer ) + 12, 100 );
		mgr->Log( Heading::formatf( "%s : %s - %s", mgr->m_userData.find( _sessionInfo->Get_Event( ) ).c_str( ), name.c_str( ), chat.c_str( ) ) );

		WSAEVENT target = mgr->m_userData.find(name);
		if( INVALID_HANDLE_VALUE != target ) // EVENT 객체는 Window Handle 이므로 비정상일땐 Windows에 정의된 INVALID HANDLE 처리로 갑니다.
		{
			//std::unordered_map<WSAEVENT, Heading::CClientSession*>::iterator targetUser;
			auto targetUser = mgr->m_sessions.find( target );
			if( mgr->m_sessions.end( ) != targetUser )
			{
				Heading::CClientSession* currentSession = targetUser->second;

				Heading::PCK_CS_Wispering returnWisper;
				std::string Nick = mgr->m_userData.find( _sessionInfo->Get_Event( ) );

				memcpy_s(returnWisper.buffer, 12, Nick.data(), Nick.size() - 1);
				memcpy_s(returnWisper.buffer, 12, chat.data(), chat.size() - 1);
				currentSession->enqueueSend(&returnWisper);
			}
		}
	}
}

// 이전 데이터 요청 들어옴
// 해당 유저 세션 전송큐에 대기시켜놓기
void EventManager::onRequestPrevious( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{
	Heading::PCK_CS_RequestPrevious* parse = static_cast<Heading::PCK_CS_RequestPrevious*>(_sendData);
}

void EventManager::onNonDefinedCallback( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{
}

void EventManager::Remove_Event( WSAEVENT _key )
{
	m_userData.Remove( _key );
	m_wsaEvents.remove( _key );
	m_sessions.erase( _key );
}

void EventManager::Recreate_EventInfo( )
{
	// 이 동작은 Session이 유지되면 Recv 되는 중간 데이터는 Event가 정상인 Session이 살아있는 동안 유지될 것으로 가정하고
	// 모든 데이터를 비우고 접속정보를 날린 뒤 재로그인 요청하는 구조입니다.
	// 실제 서비스에선 초기화 문제로 이런 방식은 힘들지도
	// 데이터를 한 번 비운다음
	FlushSend();

	// 기본 유저데이터를 다 날려버리고
	m_userData.clear();

	//for( std::unordered_map<WSAEVENT, Heading::CClientSession*>::iterator iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	for( auto iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	{
		Heading::CClientSession* currentSession = iter->second;
		currentSession->enqueueSend( new Heading::PCK_SC_RequestReset( ) ); // 문제가 있는 소켓은 전송중에 죽는걸 기대하고 재 로그인 요청을 넣습니다.
	}

	m_wsaEvents.clear(); // 한 번 비우고

	//for( std::unordered_map<WSAEVENT, Heading::CClientSession*>::iterator iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	for( auto iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	{
		WSAEVENT currentEvent = iter->first;
		m_wsaEvents.add( currentEvent ); // 문제가 있는 소켓은 지워졌으리라 기대하고 다시 추가합니다.
	}
}


void EventManager::Log( /*E_LOG_LEVEL _level,*/ std::string _log )
{
	//if( m_logLevel < _level )
	//{
		__time64_t currtime = time( NULL );

		tm formatTime = {};
		localtime_s( &formatTime, &currtime );

		std::string formatString = Heading::formatf	(
														"[ %04i.%02i.%02i-%02i:%02i:%02i ] : %s \n"
														, formatTime.tm_year + 1900
														, formatTime.tm_mon + 1
														, formatTime.tm_wday
														, formatTime.tm_hour
														, formatTime.tm_min
														, formatTime.tm_sec
														, _log.c_str( )
													);

		m_logQueue.push( formatString );
	//}
}

void EventManager::logFlush()
{
	while( !m_logQueue.empty( ) )
	{
		std::string popResult;
		if( m_logQueue.try_pop( popResult ) )
		{
			printf( popResult.c_str( ) );
		}
	}
}

uint8_t EventManager::GetEventSize( )
{
	return m_wsaEvents.size();
}

WSAEVENT* EventManager::GetEventArray( )
{
	return *m_wsaEvents;
}

//enum E_PCK_TYPE
//{
//	PCK_Shutdown,
//	PCK_Ping,
//	PCK_Pong,
//	PCK_CS_ENTER,
//	PCK_CS_EXIT,
//	PCK_CS_CHATTING,
//	PCK_CS_WISPERING,
//	PCK_SC_WISPERING,
//	PCK_CS_REQUESTPREVIOUS,
//	PCK_SC_RETURNENTER,
//	PCK_SC_OTHERSCHATTING,
//	PCK_SC_REQUESTRESET,
//	PCK_CS_MAX
//};
//typedef SendStruct<1> PCK_SessionKey;
//typedef SendStruct<1> PCK_Shutdown;
//typedef SendStruct<1> PCK_Ping;
//typedef SendStruct<1> PCK_Pong;
//typedef SendStruct<12> PCK_CS_Enter;
//typedef SendStruct<2> PCK_CS_Exit;
//typedef SendStruct<100> PCK_CS_Chatting;
//typedef SendStruct<112> PCK_CS_Wispering;
//typedef SendStruct<112> PCK_SC_Wispering;
//typedef SendStruct<8> PCK_CS_RequestPrevious;
//typedef SendStruct<8> PCK_SC_ReturnEnter;
//typedef SendStruct<120> PCK_SC_OthersChatting;
//typedef SendStruct<8> PCK_SC_RequestReset;
EventManager::EventManager( )
	: m_handler( &(onNonDefinedCallback) ) // Null일때 처리 할 포인터
{
	m_handler.AddPacketType<Heading::PCK_CS_Enter>(onEnter);
	m_handler.AddPacketType<Heading::PCK_CS_Exit>(onExit);
	m_handler.AddPacketType<Heading::PCK_CS_Chatting>(onChatting);
	m_handler.AddPacketType<Heading::PCK_CS_Wispering>(onWispering);
	m_handler.AddPacketType<Heading::PCK_CS_RequestPrevious>(onRequestPrevious);
}

EventManager::~EventManager( )
{
	Dispose();
}
