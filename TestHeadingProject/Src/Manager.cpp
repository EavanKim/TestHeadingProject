#include "psudoPCH.h"

Manager* Manager::m_instance = nullptr;

void Manager::Init( E_LOG_LEVEL _logLevel, uint64_t _selectThreadCount )
{
	if( nullptr == m_instance )
		m_instance = new Manager( _logLevel, _selectThreadCount );
}

Manager* Manager::Get( )
{
	return m_instance;
}

void Manager::Dispose( )
{
	if( nullptr != m_instance )
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

void Manager::ChattingStartUp( )
{
	m_processLive = true;

	std::thread* thread = new std::thread( Manager::Accept, this );
	m_acceptThreads.insert( std::make_pair( 50000, thread ) );
}

void Manager::Start( uint16_t _port )
{
	m_login.Set_NewAcceptPort( _port );
}

void Manager::End( uint16_t _port )
{
	m_login.Set_CloseAcceptPort( _port );
}

void Manager::Update( )
{
	while( m_processLive || !m_recvQueue.empty( ) )
	{
		server_log_flush( );

		while( !m_newSocketQueue.empty( ) )
		{
			Heading::CreatedSocketInfo info;
			if( m_newSocketQueue.try_pop( info ) )
			{
				if( m_state_count.m_maximumSession <= m_state_count.m_currentSession )
				{
					// 이미 사이즈 초과
					closesocket( info.Sock );
				}
				else
				{
					switch( info.AcceptPort )
					{
					case 50000:
						// 순회하여
						for( selectList::iterator iter = m_zone.begin( ); m_zone.end( ) != iter; ++iter )
						{
							// 비어있으면
							if( (*iter)->Check_SessionCapacity( ) )
							{
								// 추가하고
								if( (*iter)->Set_NewSession( new CChatSession( info.Sock ) ) )
								{
									++m_state_count.m_currentSession;
									break;
								}
							}
						}
						// 세션 사이즈가 남았는데도 추가가 안된거라면 셀렉트를 더 만들어도 되므로
						// 셀렉트째로 새로 만들어서 처리
						CChatter* newSelecter = new CChatter( );
						m_zone.push_back( newSelecter );
						newSelecter->Set_NewSession( new CChatSession( info.Sock ) );
						break;
					}
				}
			}
		}

		for( selectList::iterator iter = m_zone.begin( ); m_zone.end( ) != iter; ++iter )
		{
			(*iter)->Do_Select( nullptr );
		}

		while( !m_recvQueue.empty( ) )
		{
			server_log( E_LOG_LEVEL_DEBUG, "someting received" );
			Heading::SessionData* header = nullptr;
			if( m_recvQueue.try_pop( header ) )
			{
				for( Heading::CPacketHandler* proc : m_handlers )
				{
					proc->Do_Process( header );
				}
				delete header;
			}
		}

		while( !m_sendQueue.empty( ) )
		{
			server_log( E_LOG_LEVEL_DEBUG, "someting sending" );
			Heading::SessionData* header = nullptr;
			if( m_sendQueue.try_pop( header ) )
			{
				sessionMap::iterator iter = m_sessions.find( header->m_sessionKey );
				if( m_sessions.end( ) != iter )
				{
					iter->second->enqueueSend( header->m_message );
				}

				delete header;
			}
		}
	}
}

int Manager::Accept( void* _ptr )
{
	Manager* mgr = static_cast< Manager* >( _ptr );

	Heading::CAccept_Mgr login;

	while( mgr->m_processLive )
	{
		if( login.Do_Select( ) )
		{
			mgr->server_log( E_LOG_LEVEL::E_LOG_LEVEL_DEBUG, "Accept Someting" );
			Heading::NewSocketList list = {};

			login.Get_NewSocket( list );

			for( Heading::CreatedSocketInfo info : list )
			{
				mgr->server_log( E_LOG_LEVEL::E_LOG_LEVEL_DEBUG, "newSocket" );

				mgr->add_accepted_socket( info );
			}
		}
	}

	login.Dispose( );

	return 0;
}

void Manager::server_log( E_LOG_LEVEL _level, std::string _log )
{
	if( m_logLevel < _level )
	{
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
	}
}

void Manager::server_log_flush( )
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

void Manager::add_accepted_socket( Heading::CreatedSocketInfo& _socket )
{
	m_newSocketQueue.push( _socket );
}

bool Manager::try_set_new_session( Heading::CreatedSocketInfo& _info )
{
	if( m_state_count.m_currentSession < m_state_count.m_maximumSession )
	{
		m_newSocketQueue.push( _info );
		++m_state_count.m_currentSession;

		return true;
	}
	else
	{
		closesocket( _info.Sock );
		return false;
	}
}

Manager::Manager( E_LOG_LEVEL _logLevel, uint64_t _selectThreadCount )
	: m_logLevel( _logLevel )
{
	std::string str;
	Heading::WSAErrorString( WSAStartup( MAKEWORD( 2, 2 ), &m_data ), str );
	server_log( E_LOG_LEVEL::E_LOG_LEVEL_DEBUG, Heading::formatf( "Heading::string Error %s \n", str.c_str( ) ) );

	m_state_count.m_acceptThread = 1;
	m_state_count.m_selectThread = _selectThreadCount;

	m_state_count.m_maximumSession = ( m_state_count.m_selectThread * 64 );

	InterlockedExchange64( &m_managerWork, 1 );
}

Manager::~Manager( )
{
	m_processLive = false;

	for(std::unordered_map<uint16_t, std::thread*>::iterator iter = m_acceptThreads.begin(); m_acceptThreads.end() != iter; ++iter )
	{
		if( iter->second->joinable( ) )
			iter->second->join( );

		delete iter->second;
	}

	m_acceptThreads.clear( );

	for(std::unordered_map<WSAEVENT, std::thread*>::iterator iter = m_selectThreads.begin(); m_selectThreads.end() != iter; ++iter )
	{
		if( iter->second->joinable( ) )
			iter->second->join( );

		delete iter->second;
	}

	m_selectThreads.clear( );

	for( std::thread* thread : m_backgroundThreads )
	{
		if( thread->joinable( ) )
			thread->join( );

		delete thread;
	}

	m_backgroundThreads.clear( );

	for(selectList::iterator iter = m_zone.begin(); m_zone.end() != iter; ++iter )
	{
		delete (*iter);
	}

	m_zone.clear( );

	for( Heading::CPacketHandler* proc : m_handlers )
	{
		delete proc;
	}

	m_handlers.clear( );

	while( !m_sendQueue.empty( ) )
	{
		Heading::SessionData* sendData;
		if( m_sendQueue.try_pop( sendData ) )
		{
			// 소멸자에서 처리하는게 더 안전해질 것이므로 이동하기.
			// 안전처리상 잠시 방치..
			// 더블프리하면 터질 것을 기대합니다.
			delete sendData->m_message;
			delete sendData;
		}
	}

	while( !m_newSocketQueue.empty( ) )
	{
		Heading::CreatedSocketInfo info;
		if( m_newSocketQueue.try_pop( info ) )
		{
			closesocket(info.Sock);
		}
	}

	WSACleanup( );
}
