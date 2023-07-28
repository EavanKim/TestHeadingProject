#include "psudoPCH.h"

CChatUser::CChatUser( )
{
}

CChatUser::~CChatUser( )
{
	clear();
}

void CChatUser::Add( WSAEVENT _event, std::string _nickName )
{
	m_eventMap.insert( std::make_pair( _event, _nickName ) );
	m_nicknameMap.insert( std::make_pair( _nickName, _event ) );
}

void CChatUser::Remove( WSAEVENT _event )
{
	auto eventIter = m_eventMap.find( _event );
	if( m_eventMap.end( ) != eventIter )
	{
		auto nickNameIter = m_nicknameMap.find( eventIter->second );
		if( m_nicknameMap.end( ) != nickNameIter )
		{
			m_nicknameMap.erase( nickNameIter );
		}

		m_eventMap.erase( eventIter );
	}
}

void CChatUser::Remove( std::string _nickName )
{
	auto nickNameIter = m_nicknameMap.find( _nickName );
	if( m_nicknameMap.end( ) != nickNameIter )
	{
		auto eventIter = m_eventMap.find( nickNameIter->second );
		if( m_eventMap.end( ) != eventIter )
		{
			m_eventMap.erase( eventIter );
		}

		m_nicknameMap.erase( nickNameIter );
	}
}

void CChatUser::clear( )
{
	m_eventMap.clear();
	m_nicknameMap.clear();
}
