#include "psudoPCH.h"

CChatter::CChatter( )
{

}

CChatter::~CChatter( )
{

}

void CChatter::Set_NewSession( Heading::NewSocketList& _newSocket )
{
	for( Heading::CreatedSocketInfo& info : _newSocket )
	{
		if( WSA_MAXIMUM_WAIT_EVENTS > m_size )
		{
			switch( info.AcceptPort )
			{
			case 50000:
			{
				Heading::CClientSession* newSession = new CChatSession( info.Sock );
				newSession->CreateAndSetEvent( ( long ) ( FD_READ | FD_WRITE | FD_CLOSE ) );
				m_events[ m_size ] = newSession->Get_Event( );
				m_sessions.insert( std::make_pair( newSession->Get_Event( ), newSession ) );
				++m_size;
			}
			break;
			}
		}
		else
			return;
	}
}


void CChatter::Do_PostProcess( )
{
	Heading::ChatSessionEventMap::iterator iter = m_sessions.begin( );
	for( ; m_sessions.end( ) != iter; ++iter )
	{
		CChatSession* session = ( CChatSession* ) iter->second;
		Heading::packetBuff buffer;
		session->GetChatData( buffer );
		for( Heading::Header* head : buffer )
			m_chat.InsertDatas( ( uint64_t ) iter->first, head );
	}

	Heading::ChatSessionEventMap::iterator iter2 = m_sessions.begin( );
	for( ; m_sessions.end( ) != iter2; ++iter2 )
	{
		Heading::packetBuff senddata = {};
		CChatSession* session = ( CChatSession* ) iter2->second;
		m_chat.GetDatas( ( uint64_t ) iter2->first, senddata );

		session->SetChatData( senddata );
		session->SendData( );
	}
}

