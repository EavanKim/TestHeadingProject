#include "psudoPCH.h"

TestSock_Server_Select::TestSock_Server_Select( uint64_t _port )
	: m_port( _port )
{
}

TestSock_Server_Select::~TestSock_Server_Select()
{
	Dispose();
}

int TestSock_Server_Select::CreateInitializeData()
{
	addrinfo createData = {};

	SecureZeroMemory( ( PVOID )&createData, sizeof( addrinfo ) );

	createData.ai_family = AF_INET;
	createData.ai_socktype = SOCK_STREAM;
	createData.ai_protocol = IPPROTO_TCP;
	createData.ai_flags = AI_PASSIVE;

	int result = getaddrinfo( nullptr, "50000", &createData, &m_info );
	if( S_OK != result || NULL == m_info )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();

		return -1;
	}
	sockaddr_storage storage;
	memset( &storage, 0, sizeof storage );
	// The socket address to be passed to bind
	m_listenIn.sin_family = AF_INET;
	m_listenIn.sin_addr.s_addr = htonl( INADDR_ANY );
	m_listenIn.sin_port = htons( m_port );

	return 0;
}

void TestSock_Server_Select::ListenBind()
{
	// 새로 바인딩하면 초기화해버리기
	if( INVALID_SOCKET != m_bind )
	{
		closesocket( m_bind );
		m_bind = INVALID_SOCKET;
	}

	int result = 0;
	int loopCounter = 0;
	do
	{
		if( 5 < loopCounter )
		{
			int winerror = GetLastError();
			// exception 객체 생성되면 throw하면서 에러 정보 송신
			return;
		}

		m_bind = socket( m_info->ai_family, m_info->ai_socktype, m_info->ai_protocol );
		if( INVALID_SOCKET == m_bind )
		{
			continue;
		}

		result = bind( m_bind, ( SOCKADDR* )&m_listenIn, sizeof( m_listenIn ) );
		if( result == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( m_bind );
				m_bind = INVALID_SOCKET;
				continue;
			}
			printf( "connect failed with error: %d\n", err );
			freeaddrinfo( m_info );
			closesocket( m_bind );
			return;
		}
	}
	while( S_OK != result );

	if( SOCKET_ERROR == listen( m_bind, 5 ) )
		return;

	FD_ZERO( &m_readfds );
	FD_SET( m_bind, &m_readfds );
}

void TestSock_Server_Select::Select()
{
	fd_set tempfds = m_readfds;
	uint64_t fd_num = select( 0, &tempfds, NULL, NULL, NULL );
	
	for( uint64_t count = 0; tempfds.fd_count > count; ++count)
	{
		SOCKET currSock = tempfds.fd_array[ count ];

		if( FD_ISSET( currSock, &tempfds ) )
		{
			if( currSock == m_bind )
			{
				CreateNewSession();
			}
			else
			{
				Do( currSock );
			}
		}
	}
}

void TestSock_Server_Select::CreateNewSession()
{
	CSession* newSession = new CSession( accept( m_bind, NULL, NULL ), &m_log );
	if( newSession->IsConnected() )
	{
		CSession::SelectRegister( m_sessionMap, m_readfds, newSession );
	}
	else
	{
		delete newSession;
	}
}

void TestSock_Server_Select::Do( SOCKET _sock )
{
	std::unordered_map<SOCKET, CSession*>::const_iterator iter = m_sessionMap.find(_sock);
	if( m_sessionMap.end() != iter )
	{
		// Event Signal로 Recv 진행
		iter->second->EventPulse();
	}
	else
	{
		// 무언가 잘못되었습니다.
		// 터져라!
		throw -1;
	}
}

void TestSock_Server_Select::Update()
{
	Select();

	// 마지막에 로그를 모두 뿌립니다.
	m_log.FlushLog();
}

void TestSock_Server_Select::Dispose()
{
	// 재연결을 받을 수 있는 내용부터 처리합니다.
	if( INVALID_SOCKET != m_bind )
	{
		closesocket( m_bind );
		m_bind = INVALID_SOCKET;
	}

	freeaddrinfo( m_info );

	for( std::pair<SOCKET, CSession*> session : m_sessionMap )
	{
		CSession::SelectUnregister( m_sessionMap, m_readfds, session.second );
	}

	m_sessionMap.clear();

	// 남은 로그도 인정사정없이 모두 출력합니다.
	m_log.FlushLog();
}

