#include <stdint.h>
#include <time.h>
#include <iostream>

#include "define.h"
#include "CSession.h"
#include "TestSock.h"

TestSock_Server::TestSock_Server( uint64_t _port )
	: m_port( _port )
{
}

TestSock_Server::~TestSock_Server()
{
	if( INVALID_SOCKET != m_listenSock )
	{
		closesocket( m_listenSock );
		m_listenSock = INVALID_SOCKET;
	}

	for( CSession* session : m_sessionList )
	{
		delete session;
	}

	m_sessionList.clear();
}

void TestSock_Server::CreateInitializeData()
{
	addrinfo createData = {};

	SecureZeroMemory( ( PVOID )&createData, sizeof( addrinfo ) );

	createData.ai_family = AF_INET;
	createData.ai_socktype = SOCK_STREAM;
	createData.ai_protocol = IPPROTO_TCP;
	createData.ai_flags = AI_PASSIVE;

	int result = getaddrinfo( nullptr, "50000", &createData, &m_info );
	if( S_OK != result )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();

		return;
	}
	sockaddr_storage storage;
	memset( &storage, 0, sizeof storage );
	// The socket address to be passed to bind
	m_service.sin_family = AF_INET;
	m_service.sin_addr.s_addr = htonl( INADDR_ANY );
	m_service.sin_port = htons( m_port );
}

void TestSock_Server::Binding()
{
	// 새로 바인딩하면 초기화해버리기
	if( INVALID_SOCKET != m_listenSock )
	{
		closesocket( m_listenSock );
		m_listenSock = INVALID_SOCKET;
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

		m_listenSock = socket( m_info->ai_family, m_info->ai_socktype, m_info->ai_protocol );
		if( INVALID_SOCKET == m_listenSock )
		{
			continue;
		}

		result = bind( m_listenSock, ( SOCKADDR* )&m_service, sizeof( m_service ) );
		if( result == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( m_listenSock );
				m_listenSock = INVALID_SOCKET;
				continue;
			}
			printf( "connect failed with error: %d\n", err );
			freeaddrinfo( m_info );
			closesocket( m_listenSock );
			return;
		}
	}
	while( S_OK != result );
}

CSession* TestSock_Server::Wating()
{
	int result = listen( m_listenSock, SOMAXCONN );
	if( result == SOCKET_ERROR )
	{
		printf( "listen failed with error: %d\n", WSAGetLastError() );
		closesocket( m_listenSock );
		WSACleanup();
		return nullptr;
	}
	
	CSession* returnResult = new CSession( accept( m_listenSock, NULL, NULL ) );
	if( !returnResult->IsConnected() )
	{
		printf( "accept failed with error: %d\n", WSAGetLastError() );
		closesocket( m_listenSock );
		delete returnResult;

		WSACleanup();
		return nullptr;
	}

	m_sessionList.push_back( returnResult );

	return returnResult;
}

void TestSock_Server::CloseSession( CSession* _close )
{
	std::vector<CSession*>::const_iterator iter;
	std::vector<CSession*>::const_iterator end = m_sessionList.end();
	for( ; end != iter; ++iter )
	{
		if( *iter == _close )
		{
			break;
		}
	}

	if( end != iter )
		m_sessionList.erase( iter );

	delete _close;
}
