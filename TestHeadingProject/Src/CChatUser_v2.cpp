#include "psudoPCH.h"

CChatUser_v2::CChatUser_v2( )
{
	for( uint64_t count = 0; WSA_MAXIMUM_WAIT_EVENTS > count; ++count )
	{
		m_freeSessionKey.push(count);
	}
}

CChatUser_v2::~CChatUser_v2( )
{
	clear();
}

bool CChatUser_v2::Add( WSAEVENT _event, std::string _nickName, Heading::CSimpleSocket* _socket )
{
	if( !m_freeSessionKey.empty( ) )
	{
		uint64_t key = m_freeSessionKey.front( );
		m_freeSessionKey.pop( );

		m_evtToKeyMap.insert( std::make_pair( _event, key ) );
		m_nameToKeyMap.insert( std::make_pair( _nickName, key ) );
		m_wsaEvents.add( _event );

		return true;
	}

	return false;
}

void CChatUser_v2::Remove( WSAEVENT _event )
{
	EventToKeyMap::iterator findtarget = m_evtToKeyMap.find( _event );
	if( m_evtToKeyMap.end( ) != findtarget )
	{
		uint64_t key = findtarget->second;
		CChatSession_v2* session = m_sessionArray[ key ];

		if( nullptr != session )
		{
			m_nameToKeyMap.erase( session->getName( ) );
			m_evtToKeyMap.erase( _event );
			m_wsaEvents.remove( _event );

			delete session;
			m_sessionArray[ key ] = nullptr;
		}
	}
}

void CChatUser_v2::Remove( std::string _nickName )
{
	NameToKeyMap::iterator findtarget = m_nameToKeyMap.find(_nickName);
	if( m_nameToKeyMap.end( ) != findtarget )
	{
		uint64_t key = findtarget->second;
		CChatSession_v2* session = m_sessionArray[ key ];

		if( nullptr != session )
		{
			WSAEVENT curEvent = session->getEvent( );

			m_nameToKeyMap.erase( _nickName );
			m_evtToKeyMap.erase( curEvent );
			m_wsaEvents.remove( curEvent );

			delete session;
			m_sessionArray[ key ] = nullptr;
		}
	}
}

void CChatUser_v2::Remove( uint64_t _sessionKey )
{
	// 일단 무조건 종료를 맨 바닥으로 내려서 추적을 우선합니다.
	if( WSA_MAXIMUM_WAIT_EVENTS >= _sessionKey )
	{
		CChatSession_v2* session = m_sessionArray[_sessionKey];

		if( nullptr != session )
		{
			WSAEVENT curEvent = session->getEvent();

			m_nameToKeyMap.erase( session->getName() );
			m_evtToKeyMap.erase( curEvent  );
			m_wsaEvents.remove( curEvent );

			delete m_sessionArray[ _sessionKey ];
			m_sessionArray[ _sessionKey ] = nullptr;
		}
	}
}

void CChatUser_v2::Remove( CChatSession_v2* _session )
{
	if( nullptr != _session )
	{
		WSAEVENT curEvent = _session->getEvent();

		uint64_t key = WSA_MAXIMUM_WAIT_EVENTS; // 제대로 초기화 안되면 터지게 만듭니다.
		EventToKeyMap::iterator findtarget = m_evtToKeyMap.find( curEvent );
		if( m_evtToKeyMap.end( ) != findtarget )
		{
			key = findtarget->second;
		}

		m_nameToKeyMap.erase( _session->getName() );
		m_evtToKeyMap.erase( curEvent  );
		m_wsaEvents.remove( curEvent );

		delete m_sessionArray[ key ];
		m_sessionArray[ key ] = nullptr;
	}
}

std::string CChatUser_v2::find( WSAEVENT _event )
{
	return std::string( );
}

WSAEVENT CChatUser_v2::find( std::string _nickname )
{
	return WSAEVENT( );
}

Heading::CSimpleSocket* CChatUser_v2::find( uint64_t _key )
{
	return nullptr;
}

bool CChatUser_v2::PreSelect( WSAEVENT* _ptr, int _count )
{
	return false;
}

void CChatUser_v2::clear( )
{
	m_nameToKeyMap.clear();
	m_wsaEvents.clear();

	for ( auto& iter : m_evtToKeyMap )
	{
		uint64_t key = iter.second;

		delete m_sessionArray[key];
		m_sessionArray[key] = nullptr;
		m_freeSessionKey.push(key);
	}
}
