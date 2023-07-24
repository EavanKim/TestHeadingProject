#include "psudoPCH.h"

EventManager*	EventManager::m_instance = nullptr;
uint16_t		EventManager::m_sessionSize = 0;

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

	m_userTable.clear();
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

void EventManager::onSelect( )
{
	while( !m_acceptedSocket.empty( ) )
	{
		Heading::CClientSession* session = nullptr;
		if( m_acceptedSocket.try_pop( session ) )
		{
			m_wsaEvents.Add( session->Get_Event() );
			m_sessions.Insert();
		}
	}
}

void EventManager::onRecv( )
{
}

// 이건 언제 어디서건 EventManager를 통해 session에 접근하여 전송준비
void EventManager::onSend( IN Heading::Header* _sendData )
{
}

void EventManager::onEnter( WSAEVENT _sock )
{
}

void EventManager::onExit( WSAEVENT _sock )
{
}

void EventManager::onChatting( )
{
}

void EventManager::onWispering( )
{
}

EventManager::EventManager( )
{
}

EventManager::~EventManager( )
{
}
