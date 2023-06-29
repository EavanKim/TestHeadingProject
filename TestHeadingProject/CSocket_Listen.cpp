#include "psudoPCH.h"

CSocket_Listen::CSocket_Listen( uint64_t _ndfnum, uint16_t _port )
	: CSocket( "", "" ) // Listen 상태에선 아이피 정보가 없습니다. Any로 Bind되기 때문입니다.
	, m_listenPort( _port )
	, m_ndfnum( _ndfnum )
{

}

CSocket_Listen::~CSocket_Listen()
{
	C_Disconnect();
}

bool CSocket_Listen::ListenBind()
{
	// 새로 바인딩하면 초기화해버리기
	if( INVALID_SOCKET != m_socket )
	{
		closesocket( m_socket );
		m_socket = INVALID_SOCKET;
	}

	int result = 0;
	int loopCounter = 0;
	do
	{
		if( 5 < loopCounter )
		{
			int winerror = GetLastError();
			// exception 객체 생성되면 throw하면서 에러 정보 송신
			return false;
		}

		m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if( INVALID_SOCKET == m_socket )
		{
			continue;
		}

		result = bind( m_socket, ( SOCKADDR* )&m_listenIn, sizeof( m_listenIn ) );
		if( result == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( m_socket );
				m_socket = INVALID_SOCKET;
				continue;
			}
			printf( "connect failed with error: %d\n", err );
			closesocket( m_socket );
			return false;
		}
	}
	while( S_OK != result );

	if( SOCKET_ERROR == listen( m_socket, 5 ) )
		return false;

	FD_ZERO( &m_readfds );
	FD_SET( m_socket, &m_readfds );

	return true;
}

bool CSocket_Listen::C_Connect()
{
	CreateInitializeData();
	return ListenBind();
}

int CSocket_Listen::C_Send( Header* _data )
{
	printf("Called CSocket_Listen::C_Send is empty");
	return 0;
}

int CSocket_Listen::C_Recv()
{
	printf( "Called CSocket_Listen::C_Recv is empty" );
	return 0;
}

bool CSocket_Listen::C_Disconnect()
{
	for( std::pair<SOCKET, CSession*> session : m_sessionMap )
	{
		CSession::SelectUnregister( m_sessionMap, m_readfds, session.second );
	}

	m_sessionMap.clear();

	FD_ZERO( &m_readfds );

	return CSocket::C_Disconnect();
}

// Listen과 일반 소켓의 초기화는 서로 상이하므로 상위 객체의 함수를 호출하면 안됩니다.
void CSocket_Listen::CreateInitializeData()
{
	sockaddr_storage storage;
	memset( &storage, 0, sizeof storage );
	// The socket address to be passed to bind
	m_listenIn.sin_family = AF_INET;
	m_listenIn.sin_addr.s_addr = htonl( INADDR_ANY );
	m_listenIn.sin_port = htons( m_listenPort );
}

void CSocket_Listen::Select( std::vector<CMessage>& _receiveDatas )
{
	struct timeval tv, tv1;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	fd_set tempfds = m_readfds;
	uint64_t fd_num = select( m_ndfnum, &tempfds, NULL, NULL, &tv1 );

	if( 0 != fd_num )
	{
		for( uint64_t count = 0; tempfds.fd_count > count; ++count )
		{
			SOCKET currSock = tempfds.fd_array[ count ];

			if( FD_ISSET( currSock, &tempfds ) )
			{
				if( currSock == m_socket )
				{
					CreateNewSession();
				}
				else
				{
					Do( currSock, _receiveDatas );
				}
			}
		}
	}
}

void CSocket_Listen::CreateNewSession()
{
	CSession* newSession = new CSession( accept( m_socket, NULL, NULL ), NULL );
	if( newSession->IsConnected() )
	{
		CSession::SelectRegister( m_sessionMap, m_readfds, newSession );
	}
	else
	{
		delete newSession;
	}
}

void CSocket_Listen::Do( SOCKET _sock, std::vector<CMessage>& _receiveDatas )
{
	std::unordered_map<SOCKET, CSession*>::const_iterator iter = m_sessionMap.find( _sock );
	if( m_sessionMap.end() != iter )
	{
		// PrintOut 객체가 들어가면 자체 스레드 동작
		// 아니라면 바로 Process
		iter->second->Work( _receiveDatas );
	}
	else
	{
		// 무언가 잘못되었습니다.
		// 터져라!
		throw - 1;
	}
}

void CSocket_Listen::BroadCasting( CMessage* _data )
{
	std::unordered_map<SOCKET, CSession*>::iterator Enditer = m_sessionMap.end();
	for( std::unordered_map<SOCKET, CSession*>::iterator iter = m_sessionMap.begin(); Enditer != iter; ++iter )
	{
		// CMessage_BroadCast는 데이터를 보낸 대상의 sessionKey를 들고있거나 빈 키를 들고있는데
		// 비어있으면 모든 대상에게 보내고, 세션키가 지정되었다면 해당 키를 제외한 나머지에게 발송합니다.
		if( !iter->second->CheckSessionKey( _data->m_sessionKey ) )
		{
			// Send
			iter->second->Send( _data->m_message );
		}
	}
}
