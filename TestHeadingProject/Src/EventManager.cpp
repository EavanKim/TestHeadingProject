#include "psudoPCH.h"

EventManager*											EventManager::m_instance		= nullptr;
uint16_t												EventManager::m_sessionSize		= 0;
concurrency::concurrent_queue<Heading::CClientSession*>	EventManager::m_acceptedSocket;

void EventManager::init( E_PCK_CS_TYPE _type )
{
	if( nullptr == m_instance )
		m_instance = new EventManager( _type );
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
	// �������� ���� ��ü�� ���� ����
	m_userData.clear();
	// m_wsaEvents - �̰� �׳� �����ع�����.

	// ť �����̳ʵ� ����� �� pop ó���մϴ�.
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

	// ������ ó�� ���� �� ���� ���� ��� �α׸� ����
	logFlush();
}

void EventManager::onAccept( SOCKET _sock )
{
	EventManager::get()->Log( Heading::formatf( "onAccept : %lld", _sock ) );
	// ������ �ܼ��� ä�� ���Ǹ��� ���� ������ ��

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
			Heading::packetBuff buff; // local buffer�ϱ� ������Ŷ�� ���� Ŭ���� ���� �ʽ��ϴ�.
			current->RecvData( OUT buff );

			while( !buff.empty( ) )
			{
				Heading::Header* packet = buff.front();
				if( nullptr != packet )
				{
					if( current->CheckLive( ) ) // �޴� �� ������ ������ Ȯ�εǸ� ť�� ������ Recv �̺�Ʈ ó���� ���մϴ�.
					{
						onRecv( current, packet );
					}

					delete packet;

					buff.pop();
				}
				else
				{
					// null �� ũ���� ������� ������ ���� ũ���ý�ŵ�ϴ�.
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

// ����� ���߿� onSend ���� �� �����͸� ��ȣȭ �ϴµ� ������� ��� �� ����
// �� Ÿ�ֿ̹� �� ���� ������ ������...?
void EventManager::onSend( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{

}

// ���� ������ ��û ����
// �ش� ���� ���� ����ť�� �����ѳ���
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
	// �� ������ Session�� �����Ǹ� Recv �Ǵ� �߰� �����ʹ� Event�� ������ Session�� ����ִ� ���� ������ ������ �����ϰ�
	// ��� �����͸� ���� ���������� ���� �� ��α��� ��û�ϴ� �����Դϴ�.
	// ���� ���񽺿��� �ʱ�ȭ ������ �̷� ����� ��������
	// �����͸� �� �� ������
	FlushSend();

	// �⺻ ���������͸� �� ����������
	m_userData.clear();

	//for( std::unordered_map<WSAEVENT, Heading::CClientSession*>::iterator iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	for( auto iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	{
		Heading::CClientSession* currentSession = iter->second;
		currentSession->enqueueSend( new Heading::PCK_SC_RequestReset(  ) ); // ������ �ִ� ������ �����߿� �״°� ����ϰ� �� �α��� ��û�� �ֽ��ϴ�.
	}

	m_wsaEvents.clear(); // �� �� ����

	//for( std::unordered_map<WSAEVENT, Heading::CClientSession*>::iterator iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	for( auto iter = m_sessions.begin( ); m_sessions.end( ) != iter; ++iter )
	{
		WSAEVENT currentEvent = iter->first;
		m_wsaEvents.add( currentEvent ); // ������ �ִ� ������ ������������ ����ϰ� �ٽ� �߰��մϴ�.
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

// ���� �� ������ �������� ����
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

// ���� Ż��
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

// ä�� ����
// �ٷ� �ٸ� ���� ���� ����ť�� �����ѳ���
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

// �ӼӸ� ����
// �ٷ� ��� ���� ���� ����ť�� �����ѳ���
void EventManager::onWispering( IN Heading::CClientSession* _sessionInfo, IN Heading::Header* _sendData )
{
	EventManager* mgr = EventManager::get();

	if( nullptr != mgr )
	{
		Heading::PCK_CS_Wispering* parse = static_cast<Heading::PCK_CS_Wispering*>(_sendData);
		std::string name( 13, '\0' );// �г��� Ǯ ������ 12
		memcpy_s( name.data( ), 12, parse->buffer, 12 );
		std::string chat( 101, '\0' ); // ä�� Ǯ ������ 100
		memcpy_s( chat.data( ), 100, ( ( char* ) parse->buffer ) + 12, 100 );
		mgr->Log( Heading::formatf( "%s : %s - %s", mgr->m_userData.find( _sessionInfo->Get_Event( ) ).c_str( ), name.c_str( ), chat.c_str( ) ) );

		WSAEVENT target = mgr->m_userData.find(name);
		if( INVALID_HANDLE_VALUE != target ) // EVENT ��ü�� Window Handle �̹Ƿ� �������϶� Windows�� ���ǵ� INVALID HANDLE ó���� ���ϴ�.
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

uint8_t EventManager::GetEventSize( )
{
	return m_wsaEvents.size();
}

WSAEVENT* EventManager::GetEventArray( )
{
	return *m_wsaEvents;
}

EventManager::EventManager( E_PCK_CS_TYPE _type )
	: m_handler( &(onNonDefinedCallback) ) // Null�϶� ó�� �� ������
{
	switch( _type )
	{
		case E_EventManager_State::EventManager_Client:
			InitializeClient();
			break;
		case E_EventManager_State::EventManager_Server:
			InitializeServer();
			break;
	}

	m_handler.AddPacketType<Heading::PCK_Pong>(onPong);
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
