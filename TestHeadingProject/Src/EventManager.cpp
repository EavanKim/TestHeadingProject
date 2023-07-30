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
				m_wsaEvents.Add( newEvent );
				m_sessions.insert( std::make_pair( newEvent, ptr ) );
			}
			else
			{
				delete ptr;
			}
		}
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

			for( Heading::Header* packet : buff )
			{
				onRecv( current, packet );

				delete packet;
			}
		}

		if( NetworkEvents.lNetworkEvents & FD_WRITE )
		{
			if( NetworkEvents.lNetworkEvents & FD_WRITE
				&& NetworkEvents.iErrorCode[ FD_WRITE_BIT ] != 0 )
			{
				return;
			}
			current->SendData();
		}

		if( NetworkEvents.lNetworkEvents & FD_CLOSE )
		{
			m_sessions.erase( iter );
			m_wsaEvents.Remove( currentEvent );
			m_userData.Remove( currentEvent );
			delete current;
			return;
		}
	}
}

void EventManager::onRecv( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _recvData )
{
	switch( _recvData->type )
	{
		case E_PCK_TYPE::PCK_CS_ENTER:
			{
				Heading::PCK_CS_Enter* parse = static_cast< Heading::PCK_CS_Enter* >( _recvData );
				onEnter( _sessionInfo, parse );
			}
			break;
		case E_PCK_TYPE::PCK_CS_EXIT:
			onExit( _sessionInfo );
			break;
		case E_PCK_TYPE::PCK_CS_CHATTING:
			{
				Heading::PCK_CS_Chatting* parse = static_cast< Heading::PCK_CS_Chatting* >( _recvData );
				onChatting( _sessionInfo, parse );

				// echo
				{
					for (size_t i = 0; i < 500; ++i)
					{
						auto echoPacket = new std::remove_pointer_t<decltype(parse)>;
						memcpy(echoPacket, parse, parse->length);
						_sessionInfo->enqueueSend(echoPacket);
					}
					_sessionInfo->SendData();
				}
			}
			break;
		case E_PCK_TYPE::PCK_CS_WISPERING:
			{
				Heading::PCK_CS_Wispering* parse = static_cast< Heading::PCK_CS_Wispering* >( _recvData );
				onWispering( _sessionInfo, parse );
			}
			break;
		case E_PCK_TYPE::PCK_CS_REQUESTPREVIOUS:
			{
				Heading::PCK_CS_RequestPrevious* parse = static_cast< Heading::PCK_CS_RequestPrevious* >( _recvData );
				onRequestPrevious( _sessionInfo, parse );
			}
			break;
	}
}

// 여기는 나중에 onSend 들어갔을 때 데이터를 암호화 하는데 써야할지 고민 해 보기
// 이 타이밍에 할 일이 무엇이 있을까...?
void EventManager::onSend( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{

}

// 접속 한 유저의 계정정보 설정
void EventManager::onEnter( IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_Enter* _recvData )
{
	Log( Heading::formatf( "Enter : %s", _recvData->buffer ) );
	m_userData.Add(_sessionInfo->Get_Event(), _recvData->buffer);
}

// 유저 탈출
void EventManager::onExit( IN Heading::CClientSession* _sessionInfo )
{
	WSAEVENT targetEvent = _sessionInfo->Get_Event( );
	std::string nickName = m_userData.find( targetEvent );
	Log( Heading::formatf( "Exit : %s", nickName ) );
	m_userData.Remove( targetEvent );
	m_sessions.erase( targetEvent );
	m_wsaEvents.Remove( targetEvent );
	delete _sessionInfo;
}

// 채팅 받음
// 바로 다른 유저 세션 전송큐에 대기시켜놓기
void EventManager::onChatting( IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_Chatting* _sendData )
{
	Log( Heading::formatf( "%s : %s", m_userData.find(_sessionInfo->Get_Event()).c_str(), _sendData->buffer ) );
}

// 귓속말 받음
// 바로 대상 유저 세션 전송큐에 대기시켜놓기
void EventManager::onWispering( IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_Wispering* _sendData )
{
}

// 이전 데이터 요청 들어옴
// 해당 유저 세션 전송큐에 대기시켜놓기
void EventManager::onRequestPrevious( IN Heading::CClientSession* _sessionInfo, IN Heading::PCK_CS_RequestPrevious* _sendData )
{
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

EventManager::EventManager( )
{

}

EventManager::~EventManager( )
{
	Dispose();
}
