// TestHeadingProject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>

#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>

uint64_t m_resultsize = 0;
char Buffer[ 1 << 13 ];
char RecvBuffer[ 1 << 13 ];

void WriteResultBuffer( char* _buffer, unsigned long long _length )
{
	printf("[Input debug buffer : %s][debug ptr : %llX] \n", _buffer, _buffer );
	printf("[Input debug length : %lld] \n", _length );

	char* destPtr = ( char* )( Buffer + m_resultsize );
	unsigned long long length = ( unsigned long long )( 1 << 13 ) - m_resultsize;
	memcpy_s( destPtr, length, _buffer, _length );

	m_resultsize += _length;
}

int main()
{
	uint64_t m_bufferSize = 0;

	uint64_t m_currentpacketSize = 0;
	uint64_t m_totalRecv = 0;

	WSADATA m_data = {};
	addrinfo* m_info = nullptr;
	SOCKET m_socket = INVALID_SOCKET;

	int result = WSAStartup( MAKEWORD( 2, 2 ), &m_data );
	if( S_OK != result )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();
		// exception 객체 생성되면 throw하면서 에러 정보 송신
		return 1;
	}

	addrinfo createData = {};

	SecureZeroMemory( ( PVOID )&createData, sizeof( addrinfo ) );

	createData.ai_family = AF_INET;
	createData.ai_socktype = SOCK_STREAM;
	createData.ai_protocol = IPPROTO_TCP;
	createData.ai_flags = AI_PASSIVE;

	result = getaddrinfo( nullptr, "50000", &createData, &m_info );
	if( S_OK != result )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();
		// exception 객체 생성되면 throw하면서 에러 정보 송신
		return 1;
	}
	sockaddr_storage storage;
	memset( &storage, 0, sizeof storage );
	// The socket address to be passed to bind
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl( INADDR_ANY );
	service.sin_port = htons( 50000 );


	int loopCounter = 0;
	do
	{
		if( 5 < loopCounter )
		{
			int winerror = GetLastError();

			if( INVALID_SOCKET != m_socket )
			{
				closesocket( m_socket );
				m_socket = INVALID_SOCKET;
			}
			// exception 객체 생성되면 throw하면서 에러 정보 송신
			return 1;
		}

		m_socket = socket( m_info->ai_family, m_info->ai_socktype, m_info->ai_protocol );
		if( INVALID_SOCKET == m_socket )
		{
			continue;
		}

		result = bind( m_socket, ( SOCKADDR* )&service, sizeof( service ) );
		if( result == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( m_socket );
				m_socket = INVALID_SOCKET;
				continue;
			}
			wprintf( L"connect failed with error: %d\n", err );
			freeaddrinfo( m_info );
			closesocket( m_socket );
			return 1;
		}
	}
	while( S_OK != result );

	DWORD receiveSize = 0;

	freeaddrinfo( m_info );

	result = listen( m_socket, SOMAXCONN );
	if( result == SOCKET_ERROR )
	{
		printf( "listen failed with error: %d\n", WSAGetLastError() );
		closesocket( m_socket );
		WSACleanup();
		return 1;
	}

	SOCKET sessionOpen = INVALID_SOCKET;
	sessionOpen = accept( m_socket, NULL, NULL );
	if( sessionOpen == INVALID_SOCKET )
	{
		printf( "accept failed with error: %d\n", WSAGetLastError() );
		closesocket( m_socket );
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket( m_socket );

	uint64_t counter0 = 0;
	time_t start = time(NULL);
	while( 1 )
	{
		unsigned long long reserveSize = ( ( unsigned long long )( 1 << 13 ) - m_totalRecv );
		if( 64 > reserveSize )
		{
			m_totalRecv = 0;
		}
		receiveSize = recv( sessionOpen, RecvBuffer + reserveSize, reserveSize, 0 );
		if( 0 == receiveSize )
		{
			if( 0 != m_resultsize )
			{
				time_t now = time( NULL );
				printf( "[%lld]Result[%s] \n", now - start, Buffer );

				//ZeroMemory( Buffer, sizeof( Buffer ) );
				m_resultsize = 0;
			}
			continue;
		}
		else if( -1 == receiveSize )
		{
			int sockerror = WSAGetLastError();
			int winerror = GetLastError();
			// 에러복구
			continue;
		}
		{
			time_t now = time( NULL );
			if( UINT64_MAX == counter0 )
			{
				counter0 = 0;
			}
			unsigned long long Time = now - start;
			unsigned long long Timedevide = ( 0 == Time ? 1 : Time );
			unsigned long long MPS = counter0 / Timedevide;
			printf( "[reserveSize : %lld][receiveSize : %lld][m_totalRecv : %lld] \n", reserveSize, receiveSize, m_totalRecv );
			printf( "[%lld][count : %lld][MPS : %lld]TestBuffer[%s] \n", Time, counter0, MPS, RecvBuffer + reserveSize + 16 );
			printf( "Debug BufferInfo [%s] \n", RecvBuffer );
			++counter0;
		}


		// 일단 상수로 먼저 짭니다.
		// 어차피 Sync는 테스트 용도에 가까우므로 이 이후에 제대로 작성해야합니다.
		m_totalRecv += receiveSize;
		unsigned long long* recvBufLengPtr = ( unsigned long long* )( RecvBuffer + reserveSize + 8 );
		unsigned long long BufferLength = *recvBufLengPtr;
		m_currentpacketSize = 16 + BufferLength;
		printf( "Debug MessagePtr [%llX] \n", RecvBuffer + reserveSize + 16 );
		printf( "Debug Packetptr [%llX] \n", recvBufLengPtr );
		printf( "Debug Packetlength [%lld][%llX] \n", BufferLength, BufferLength );
		printf( "Debug PacketSize [%lld][%llX] \n", m_currentpacketSize, m_currentpacketSize );
		if( m_currentpacketSize == m_totalRecv )
		{
			WriteResultBuffer( RecvBuffer + reserveSize + 16, BufferLength );
			m_totalRecv = 0;
		}
		else if( m_currentpacketSize < m_totalRecv )
		{
			WriteResultBuffer( RecvBuffer + reserveSize + 16, BufferLength );

			unsigned long long ReserveSize = m_totalRecv - m_currentpacketSize;
			memcpy( RecvBuffer + reserveSize, RecvBuffer + m_currentpacketSize, ReserveSize );

			m_totalRecv = m_totalRecv - m_currentpacketSize;
		}

		if( 0 != m_resultsize )
		{
			time_t now = time( NULL );
			printf( "[%lld]Result[%s] \n", now - start, Buffer );

			m_resultsize = 0;
		}
	}

	//================================================================================================================================================================
}