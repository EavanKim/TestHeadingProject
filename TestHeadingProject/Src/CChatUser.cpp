#include "psudoPCH.h"
#include "CChatUser.h"

void CChatUser::Add( WSAEVENT _handle, std::string _userName )
{
	m_userTable.insert( std::make_pair( _userName, _handle ) );
	m_handleTable.insert( std::make_pair( _handle, _userName ) );
}

bool CChatUser::Get( IN std::string _userName, OUT WSAEVENT& _result )
{
	auto findtarget = m_userTable.find( _userName );
	if( m_userTable.end( ) != findtarget )
	{
		_result = findtarget->second;
		return true;
	}

	_result = INVALID_HANDLE_VALUE;
	return false;
}

bool CChatUser::Get( IN WSAEVENT _handle, OUT std::string _result )
{
	auto findtarget = m_handleTable.find( _handle );
	if( m_handleTable.end( ) != findtarget )
	{
		_result = findtarget->second;
		return true;
	}

	_result = "";
	return false;
}

void CChatUser::remove( std::string _userName )
{
	auto findtarget = m_userTable.find( _userName );
	if( m_userTable.end( ) != findtarget )
	{
		m_handleTable.unsafe_erase( findtarget->second );
		m_userTable.unsafe_erase( findtarget );
	}
}

void CChatUser::remove( WSAEVENT _handle )
{
	auto findtarget = m_handleTable.find( _handle );
	if( m_handleTable.end( ) != findtarget )
	{
		m_userTable.unsafe_erase( findtarget->second );
		m_handleTable.unsafe_erase( findtarget );
	}
}

void CChatUser::clear( )
{
	// 여긴 단순히 조회 위치이므로 값에 대한 그 어떤 조작도 하지 않음.
	m_userTable.clear();
	m_handleTable.clear();
}
