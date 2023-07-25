#include "psudoPCH.h"

EventManager*													EventManager::m_instance		= nullptr;
uint16_t														EventManager::m_sessionSize		= 0;
CChatUser														EventManager::m_userData		= {};
concurrency::concurrent_queue<Heading::CClientSession*>			EventManager::m_acceptedSocket;
concurrency::concurrent_queue<std::string>						EventManager::m_logQueue;

void EventManager::init( )
{
	if( nullptr == m_instance )
		m_instance = new EventManager();
}

EventManager* EventManager::get( )
{
	return m_instance;
}

void EventManager::Dispose( )
{
	// 혹시 비어있음 외의 상황으로 try_pop에 실패하지 않도록 비어있는가 체크로 루프
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
		// 이벤트와 소켓 모두 이 때 닫힐 것.
		delete iter.second;
	}
	m_sessions.clear();

	m_userData.clear();
}

void EventManager::AddBindPort( uint16_t _port )
{
	m_accepter = new Heading::CAccepter( _port );
	m_accepter->Bind();
}

void EventManager::NewSessionProcess( )
{
	while( !m_acceptedSocket.empty( ) )
	{
		Heading::CClientSession* session = nullptr;
		if( m_acceptedSocket.try_pop( session ) )
		{
			//m_wsaEvents.Add( session->Get_Event() );
			//m_sessions.Insert();
		}
	}
}

void EventManager::onAccept( SOCKET _sock )
{
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

void EventManager::onSelect( WSAEVENT _sock )
{
	onRecv( _sock );
}

void EventManager::onRecv( WSAEVENT _sock )
{

}

// 이건 언제 어디서건 EventManager를 통해 session에 접근하여 전송준비
void EventManager::onSend( IN Heading::Header* _sendData )
{
}

void EventManager::onChatting( )
{
}

void EventManager::onWispering( )
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
														"[ %04i.%02i.%02i-%02i:%02i:%02i ] : %s"
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

EventManager::EventManager( )
{
}

EventManager::~EventManager( )
{
}
